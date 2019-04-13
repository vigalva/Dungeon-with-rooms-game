#include "GLUT.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>
#include "CompareNewNodes.h"
#include "Point2D.h"
#include "Room.h"
#include "Node.h"
#include "NewNode.h"
#include "CompareNodes.h"
#include "Parent.h"
#include "Road.h"
#include "Character.h"
using namespace std;

const int W = 600; // window width
const int H = 600; // window height
const int NUM_ROOMS = 10;

const int SPACE = 1;
const int TUNNEL = 2;
const int WALL = 3;
const int NPC1 = 4;
const int NPC2 = 5;
const int AMMO = 6;
const int HEALTH = 7;
const int DEAD = 8;

const int STARTHEALTH = 20;
const int STARTBULLETS = 0;

const int UP = 1;
const int DOWN = 2;
const int LEFT = 3;
const int RIGHT = 4;

const int MSIZE = 100;
const double SQSIZE = 2.0 / MSIZE;

const int NUMOFWEAPONANDHEALTHINROOM = 2;

int maze[MSIZE][MSIZE];
bool middle1 , middle2;
bool game = false;
Room all_rooms[NUM_ROOMS];
list<Road> all_roads;
// gray queue
vector <Point2D> gray;
vector <Point2D> black;
vector <Parent> parents;

priority_queue<Node, vector<Node>, CompareNodes> pq;
priority_queue<NewNode, vector<NewNode>, CompareNewNodes> bestNodes1;
priority_queue<Node, vector<Node>, CompareNodes> bestNodes2;
queue<Point2D, list<Point2D>> q;

list <Road> RoomPath;
list <Parent> path;
Point2D start,target;
Point2D player1, player2;
Point2D *lastExit1 = new Point2D(-1, -1);
Point2D *lastExit2 = new Point2D(-1, -1);
Point2D bad = Point2D(-1, -1);
Character npc1, npc2;
int nextPoint1, nextPoint2;
int NumOfHealthPackets = NUMOFWEAPONANDHEALTHINROOM * NUM_ROOMS;
int NumOfAmmoPackets = NUMOFWEAPONANDHEALTHINROOM * NUM_ROOMS;
void SetupMaze();
void RecordExits();
void SetNpcStartLocation();
void SetArmorAndHelathLocation();
Point2D BFSOnRooms(Room r,const int LABEL, Point2D player ,Point2D lastExitOfRival, int roomNumberOfRival);

void init()
{
	int i, j;

	srand(time(0));
	// clean up the maze
	for (i = 0; i < MSIZE; i++)
		for (j = 0; j < MSIZE; j++)
			maze[i][j] = WALL;

	
	SetupMaze();
	SetNpcStartLocation();
	SetArmorAndHelathLocation();
	

	glClearColor((GLclampf)0.7, (GLclampf) 0.7, (GLclampf)0.7, (GLclampf)0);

	glOrtho(-1, 1, -1, 1, -1, 1);
}

void AddNewNode(Node current, int direction)
{
	Node* tmp;
	Point2D* pt;
	vector<Point2D>::iterator gray_it;
	vector<Point2D>::iterator black_it;
	double space_weight = 0.1, wall_weight = 5, weight;
	int dx, dy;

	switch (direction)
	{
	case UP:
		dx = 0;
		dy = -1;
		break;
	case DOWN:
		dx = 0;
		dy = 1;
		break;
	case LEFT:
		dx = -1;
		dy = 0;
		break;
	case RIGHT:
		dx = 1;
		dy = 0;
		break;
	}// switch

	if (direction==UP && current.GetPoint().GetY() > 0 ||
		direction == DOWN && current.GetPoint().GetY() < MSIZE-1 ||
		direction == LEFT && current.GetPoint().GetX() > 0 ||
		direction == RIGHT && current.GetPoint().GetX() < MSIZE - 1)
	{
		pt = new Point2D(current.GetPoint().GetX()+dx, current.GetPoint().GetY() +dy);
		gray_it = find(gray.begin(), gray.end(), *pt);
		black_it = find(black.begin(), black.end(), *pt);
		if (gray_it == gray.end() && black_it == black.end()) // this is a new point
		{
			// very important to tunnels
			if (maze[current.GetPoint().GetY() +dy][current.GetPoint().GetX()+dx] == WALL)
				weight = wall_weight;
			else weight = space_weight;
			// weight depends on previous weight and wheater we had to dig
			// to this point or not
			tmp = new Node(*pt, target, current.GetG() + weight);
			pq.emplace(*tmp); // insert first node to priority queue
			gray.push_back(*pt); // paint it gray
			// add Parent
			parents.push_back(Parent(tmp->GetPoint(), current.GetPoint(), true));
		}
	}

}

void RunAStar4Tunnels()
{
	Node current;
	vector<Point2D>::iterator gray_it;
	vector<Point2D>::iterator black_it;
	bool finished = false;
	double space_weight = 0.5, wall_weight = 0.5;

	while (!pq.empty() && !finished)
	{
		current = pq.top();
		pq.pop(); // remove it from pq

		if (current.GetH() == 0) // the solution has been found
		{
			vector<Parent>::iterator itr;
			finished = true;
			// go back to start and change WALL to SPACE
			itr = find(parents.begin(), parents.end(),
				Parent(current.GetPoint(), current.GetPoint(), true));
			while (itr->HasParent())
			{
				Point2D tmp_prev = itr->GetPrev();
				Point2D tmp_cur = itr->GetCurrent();
				// set SPACE
				if (maze[tmp_cur.GetY()][tmp_cur.GetX()] == WALL)
					maze[tmp_cur.GetY()][tmp_cur.GetX()] = TUNNEL;
				itr = find(parents.begin(), parents.end(),
					Parent(tmp_prev, current.GetPoint(), true));
			}
		}
		else // check the neighbours
		{
			// remove current from gray 
			gray_it = find(gray.begin(), gray.end(), current.GetPoint());
			if (gray_it != gray.end())
				gray.erase(gray_it);
			// and paint it black
			black.push_back(current.GetPoint());
			// try to go UP
			AddNewNode(current, UP);
			// try to go DOWN
			AddNewNode(current, DOWN);
			// try to go LEFT
			AddNewNode(current, LEFT);
			// try to go RIGHT
			AddNewNode(current, RIGHT);

		}

	} // while
}


void DigTunnels()
{
	int i, j;

	for(i=0;i<NUM_ROOMS;i++)
		for (j = i + 1; j < NUM_ROOMS; j++)
		{
			start = all_rooms[i].GetCenter();
			target = all_rooms[j].GetCenter();

			printf("Start: %d      Target: %d\n", i, j);

			Node* tmp = new Node(start, target, 0);
			while (!pq.empty())
				pq.pop();

			pq.emplace(*tmp); // insert first node to priority queue
			gray.clear();
			gray.push_back(start); // paint it gray
			black.clear();
			parents.clear();
			parents.push_back(Parent(tmp->GetPoint(),
				tmp->GetPoint(), false));
			RunAStar4Tunnels();
			delete tmp;
		}
}

const int getLabelFromChoice(int choice)
{
	switch (choice)
	{
	case 1:
		return HEALTH;
	case 2:
		return AMMO;
	default:
		return -1;
	}
}

Point2D getExitPoint(Point2D startOfTunnel)
{
	for (list<Road>::iterator i = all_roads.begin(); i != all_roads.end(); i++)
	{
		if ((*i).GetCurrent() == startOfTunnel)
			return (*i).GetPrev();
	}
	return bad;
}

Point2D getExitFromTunnel(Point2D startOfTunnel)
{
	Point2D current = startOfTunnel;
	Point2D prev = current;
	int dx, dy;
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			dx = (i % 2) * (2 * (i / 2) - 1);
			dy = (1 - i % 2)* (2 * (i / 2) - 1);
			if (maze[current.GetY() + dy][current.GetX() + dx] == SPACE)
			{
				while (maze[current.GetY()][current.GetX()] == TUNNEL)
				{
					prev = current;
					current = *new Point2D(current.GetX() - dx, current.GetY() - dy);
					if (maze[current.GetY()][current.GetX()] == WALL)
					{
						if (maze[prev.GetY() - (i % 2)][prev.GetX() - (1 - i % 2)] == WALL)
						{
							
							dx = -(1 - i % 2);
							dy = -(i % 2);
							i = (1 - i % 2);
							current = *new Point2D(prev.GetX() - dx, prev.GetY() - dy);
						}
						else
						{
							dx = (1 - i % 2);
							dy = (i % 2);
							i = (1 - i % 2);
							current = *new Point2D(prev.GetX() - dx, prev.GetY() - dy);
						}
					}
				}
				return prev;
			}
		}
	}
	return Point2D();
}

void RecordExits()
{
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		int top, bottom, left, right;
		top = all_rooms[i].GetCenter().GetY() - all_rooms[i].GetHeight() / 2;
		if (top < 1) top = 1;
		bottom = all_rooms[i].GetCenter().GetY() + all_rooms[i].GetHeight() / 2;
		if (bottom >= MSIZE - 1) bottom = MSIZE - 2;
		left = all_rooms[i].GetCenter().GetX() - all_rooms[i].GetWidth() / 2;
		if (left < 1) left = 1;
		right = all_rooms[i].GetCenter().GetX() + all_rooms[i].GetWidth() / 2;
		if (right >= MSIZE - 1) right = MSIZE - 2;
		for (int j = left; j <= right; j++)
		{
			if (maze[top - 1][j] == TUNNEL)
			{
				all_rooms[i].InsertExitPoint(*(new Point2D(j, top - 1)));
			}
			if (maze[bottom + 1][j] == TUNNEL)
			{
				all_rooms[i].InsertExitPoint(*(new Point2D(j, bottom + 1)));
			}
		}
		for (int j = top; j <= bottom; j++)
		{
			if (maze[j][left - 1] == TUNNEL)
			{
				all_rooms[i].InsertExitPoint(*(new Point2D(left - 1, j)));
			}
			if (maze[j][right + 1] == TUNNEL)
			{
				all_rooms[i].InsertExitPoint(*(new Point2D(right + 1, j)));
			}
		}
	}

	for (int i = 0; i < NUM_ROOMS; i++)
	{
		Point2D* e = all_rooms[i].GetExitPoints();
		int num = all_rooms[i].GetNumOfExits();
		for (int j = 0; j < num; j++)
		{
			all_roads.push_back(*(new Road(e[j], getExitFromTunnel(e[j]))));
		}
	}
	all_roads.sort();
	
}

void clearQue()
{
	while (!bestNodes1.empty())
	{
		bestNodes1.pop();
	}
	while (!bestNodes2.empty())
	{
		bestNodes2.pop();
	}
	while (!pq.empty())
	{
		pq.pop();
	}
	while (!q.empty())
	{
		q.pop();
	}
	path.clear();
	RoomPath.clear();

}

int selectNewRandomTargetChoice(Character chr, Character other)
{
	int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
	int H = (100 - chr.GetHealth())* (double)((double)NumOfHealthPackets/ (double)(NUMOFWEAPONANDHEALTHINROOM * NUM_ROOMS));
	int A = (100 - chr.GetAmmo() * 3 -1) *(double)((double)NumOfAmmoPackets / (double)(NUMOFWEAPONANDHEALTHINROOM * NUM_ROOMS));
	int E = (0.7*(other.GetAmmo() - chr.GetAmmo()) + 0.3*(other.GetHealth() - chr.GetHealth())) * 10;
	int C = (0.3*(chr.GetAmmo() - other.GetAmmo()) + 0.7*(chr.GetHealth() - other.GetHealth())) * 10 + 10;
	if (H > 0)
	{
		tmp1 = rand() % H;
	}
	if (A > 0)
	{
		tmp2 = rand() % A;
	}
	if (E > 0 && other.GetChoice() == 0 && other.GetAmmo() != 0)
	{
		tmp3 = rand() % E;
	}
	if (C > 0 && chr.GetAmmo()!= 0)
	{
		tmp4 = rand() % C;
	}
	int result[] = { tmp4,tmp1,tmp2,tmp3};
	return distance(result, max_element(result, result + 4));
}

void playerGotTarget(Point2D nextStep, Character* player, Point2D target)
{
	if (maze[nextStep.GetY()][nextStep.GetX()] == HEALTH)
	{
		player->SetHealth(player->GetHealth() + 5);
		NumOfHealthPackets -= 1;
		if (*player == npc1)
		{
			printf("Player1 picked up HP he is now at %d HP\n", player->GetHealth());
		}
		else
		{
			printf("Player2 picked up HP he is now at %d HP\n", player->GetHealth());
		}
		
	}

	if (maze[nextStep.GetY()][nextStep.GetX()] == AMMO)
	{
		player->SetAmmo(player->GetAmmo() + 3);
		NumOfAmmoPackets -= 1;
		if (*player == npc1)
		{
			printf("Player1 picked up Ammo he is now at %d Ammo\n", player->GetAmmo());
		}
		else
		{
			printf("Player2 picked up Ammo he is now at %d Ammo\n", player->GetAmmo());
		}
	}

	if (nextStep == target && maze[target.GetY()][target.GetX()] != SPACE && maze[target.GetY()][target.GetX()] != TUNNEL)
		player->SetChoice(-1);
}

int GetRoomNumberOfExit(Point2D pt)
{
	for (int i = 0; i < NUM_ROOMS; i++)
	{
		Point2D *e = all_rooms[i].GetExitPoints();
		int num = all_rooms[i].GetNumOfExits();
		for (int j = 0; j < num; j++)
		{
			if (e[j] == pt)
			{
				return i;
			}
		}
	}
	return -1;
}

Point2D TargetOneRoomAway(Room r, int hunterTargetRoom, Point2D player, Point2D lastExitOfRival)
{
	clearQue();
	Point2D tmp, tmp2;
	Room room = r;
	Point2D* e;
	int num;
	int index;
	vector<Point2D> visited;
	vector<Point2D>::iterator visited_it;
	Point2D current;
	//search BFS on the rest of the rooms
	e = room.GetExitPoints();
	num = room.GetNumOfExits();
	// gets pushed in queue randomly need to fix
	for (int i = 0; i < num; i++)
	{
		if (!(hunterTargetRoom == -1 && e[i] == getExitPoint(lastExitOfRival)))
		{
			q.push(e[i]);
			RoomPath.push_back(*new Road(e[i], player));
		}
	}
	while (!q.empty())
	{
		target = q.front();
		q.pop();
		tmp = getExitPoint(target);
		index = GetRoomNumberOfExit(tmp);
		r = all_rooms[index];
		if (index != hunterTargetRoom)
		{
			return target;
		}
	}
	return player;
}

Point2D BFSOnRoomForHunted(Room r,int hunterRoom,int huntedTargetRoom, Point2D player, Point2D waitTarget, Point2D lastExitOfRival)
{
	clearQue();
	Point2D tmp, tmp2;
	Room room = r;
	if (hunterRoom == huntedTargetRoom)
	{
		int dx, dy;
		for (int i = 0; i < 4; i++)
		{
			dx = (i % 2) * (2 * (i / 2) - 1);
			dy = (1 - i % 2)* (2 * (i / 2) - 1);
			if (maze[waitTarget.GetY() + 2*dy][waitTarget.GetX() + 2*dx] == SPACE)
				return *new Point2D(waitTarget.GetX() + 2*dx, waitTarget.GetY() + 2*dy);
		}
	}
	// search inside room
	Point2D* e;
	int num;
	int index;
	vector<Point2D> visited;
	vector<Point2D>::iterator visited_it;
	Point2D current;
	//search BFS on the rest of the rooms
	e = room.GetExitPoints();
	num = room.GetNumOfExits();
	// gets pushed in queue randomly need to fix
	for (int i = 0; i < num; i++)
	{
		if (!(huntedTargetRoom == -1 && e[i] == getExitPoint(lastExitOfRival)))
		{
			q.push(e[i]);
			RoomPath.push_back(*new Road(e[i], player));
		}
	}
	while (!q.empty())
	{
		target = q.front();
		q.pop();
		tmp = getExitPoint(target);
		visited_it = find(visited.begin(), visited.end(), target);
		if (visited_it == visited.end())
		{
			visited.push_back(target);
			visited.push_back(tmp);
		}
		else
			continue;

		index = GetRoomNumberOfExit(tmp);
		r = all_rooms[index];
		if (index == huntedTargetRoom)
		{
			current = target;
			Road targetPoint;
			list<Road>::iterator iter = RoomPath.end();
			for (iter--; iter != RoomPath.begin(); --iter)
			{
				targetPoint = *iter;
				if (targetPoint.GetCurrent() == current)
				{
					if (targetPoint.GetPrev() == player)
						break;
					current = targetPoint.GetPrev();
				}
			}
			return current;
		}
			
		
		e = r.GetExitPoints();
		num = r.GetNumOfExits();
		for (int i = 0; i < num; i++)
		{
			visited_it = find(visited.begin(), visited.end(), e[i]);
			if (visited_it == visited.end() && !(huntedTargetRoom == -1 && e[i] == getExitPoint(lastExitOfRival)))
			{
				q.push(e[i]);
				RoomPath.push_back(*new Road(e[i], target));
			}

		}
	}
	return player;
}


Point2D BFSOnRooms(Room r, const int LABEL, Point2D player, Point2D lastExitOfRival, int roomNumberOfRival)
{
	clearQue();
	
	Point2D target = Point2D(-1, -1);
	Point2D tmp,tmp2;
	double minDistance = DBL_MAX;
	Room room = r;

	// search inside room
	int top, bottom, left, right;
	top = r.GetCenter().GetY() - r.GetHeight() / 2;
	if (top < 0) top = 0;
	bottom = r.GetCenter().GetY() + r.GetHeight() / 2;
	if (bottom >= MSIZE) bottom = MSIZE - 1;
	left = r.GetCenter().GetX() - r.GetWidth() / 2;
	if (left < 0) left = 0;
	right = r.GetCenter().GetX() + r.GetWidth() / 2;
	if (right >= MSIZE) right = MSIZE - 1;
	for (int i = top; i <= bottom; i++)
	{
		for (int j = left; j <= right; j++)
		{
			tmp = *new Point2D(j, i);
			if (maze[tmp.GetY()][tmp.GetX()] == LABEL && minDistance > tmp.Distance(player))
			{
				target = tmp;
				minDistance = tmp.Distance(player);
			}
		}
	}
	if (!(target == bad))
	{
		return target;
	}

	Point2D* e;
	
	int num;
	int index;
	vector<Point2D> visited;
	vector<Point2D>::iterator visited_it;
	Point2D current;
	//search BFS on the rest of the rooms
	e = room.GetExitPoints();
	num = room.GetNumOfExits();
	// gets pushed in queue randomly need to fix
	for (int i = 0; i < num; i++)
	{
		if (!(roomNumberOfRival == -1 && e[i] == getExitPoint(lastExitOfRival)))
		{
			q.push(e[i]);
			RoomPath.push_back(*new Road(e[i], player));
		}
	}
	while (!q.empty())
	{
		target = q.front();
		q.pop();
		tmp = getExitPoint(target);
		visited_it = find(visited.begin(), visited.end(), target);
		if (visited_it == visited.end())
		{
			visited.push_back(target);
			visited.push_back(tmp);
		}
		else
			continue;
		
		index = GetRoomNumberOfExit(tmp);
		r = all_rooms[index];

		int top, bottom, left, right;
		top = r.GetCenter().GetY() - r.GetHeight() / 2;
		if (top < 0) top = 0;
		bottom = r.GetCenter().GetY() + r.GetHeight() / 2;
		if (bottom >= MSIZE) bottom = MSIZE - 1;
		left = r.GetCenter().GetX() - r.GetWidth() / 2;
		if (left < 0) left = 0;
		right = r.GetCenter().GetX() + r.GetWidth() / 2;
		if (right >= MSIZE) right = MSIZE - 1;

		for (int i = top; i < bottom; i++)
		{
			for (int j = left; j < right; j++)
			{
				tmp2 = *new Point2D(j, i);
				if (maze[tmp2.GetY()][tmp2.GetX()] == LABEL)
				{
					current = target;
					Road targetPoint;
					list<Road>::iterator iter = RoomPath.end();
					for (iter--; iter != RoomPath.begin(); --iter)
					{
						targetPoint = *iter;
						if (targetPoint.GetCurrent() == current)
						{
							if (targetPoint.GetPrev() == player)
								break;
							current = targetPoint.GetPrev();
						}
					}
					return current;
				}
			}
		}
		e = r.GetExitPoints();
		num = r.GetNumOfExits();
		for (int i = 0; i < num; i++)
		{
			visited_it = find(visited.begin(), visited.end(), e[i]);
			if (visited_it == visited.end() && !(roomNumberOfRival == -1 && e[i] == getExitPoint(lastExitOfRival)))
			{
				q.push(e[i]);
				RoomPath.push_back(*new Road(e[i], target));
			}
			
		}
	}
	return bad;
}


Point2D CarefulAStar(Point2D target, Point2D player, Character *plyr, Point2D rival)
{
	if (player == target)
	{
		return target;
	}
	Point2D nextStep;
	int min = MAXINT;
	list<NewNode> nodes;
	vector<NewNode> visited;
	vector<NewNode>::iterator visited_it;
	NewNode nd = *new NewNode(player, target, 0, rival), current;
	clearQue();
	bestNodes1.push(nd);
	bool finished = false;
	path.push_back(*new Parent(nd.GetPoint(), nd.GetPoint(), false));
	while (!bestNodes1.empty() && !finished)
	{
		current = bestNodes1.top();
		bestNodes1.pop();

		//if reached target finish the route search
		if (current.GetPoint() == target)
		{
			nodes.push_back(current);
			finished = true;
		}
		//otherwise keep searching
		else
		{
			visited_it = find(visited.begin(), visited.end(), current);
			if (visited_it == visited.end())
				visited.push_back(current);
			else
				continue;
			int dx, dy;
			for (int i = 0; i < 4; i++)
			{
				dx = (i % 2) * (2 * (i / 2) - 1);
				dy = (1 - i % 2)* (2 * (i / 2) - 1);
				if (maze[current.GetPoint().GetY() + dy][current.GetPoint().GetX() + dx] != WALL
					&& maze[current.GetPoint().GetY() + dy][current.GetPoint().GetX() + dx] != NPC1
					&& maze[current.GetPoint().GetY() + dy][current.GetPoint().GetX() + dx] != NPC2)
				{
					Point2D pt = *new Point2D(current.GetPoint().GetX() + dx, current.GetPoint().GetY() + dy);
					NewNode nd = *new NewNode(pt, target, current.GetG() + 1, rival);
					path.push_back(*new Parent(pt, current.GetPoint(), true));
					bestNodes1.push(nd);
				}
				if (((rival.GetX() + dx == target.GetX() && rival.GetY() + dy == target.GetY())
					|| (rival == target)) && (maze[current.GetPoint().GetY() + dy][current.GetPoint().GetX() + dx] == NPC1
					|| maze[current.GetPoint().GetY() + dy][current.GetPoint().GetX() + dx] == NPC2))
				{
					Point2D pt = *new Point2D(current.GetPoint().GetX() + dx, current.GetPoint().GetY() + dy);
					NewNode nd = *new NewNode(pt, target, current.GetG() + 1, rival);
					path.push_back(*new Parent(pt, current.GetPoint(), true));
					bestNodes1.push(nd);
				}
			}
		}
		if (current.GetPoint() == target)
		{
			Parent targetPoint;
			list<Parent>::iterator iter = path.end();
			for (iter--; iter != path.begin(); --iter)
			{
				targetPoint = *iter;
				if (targetPoint.HasParent() && targetPoint.GetCurrent() == current.GetPoint())
				{
					if (targetPoint.GetPrev() == player)
						break;
					current = *new NewNode(targetPoint.GetPrev(), target, 0, rival);
				}
			}
			nextStep = targetPoint.GetCurrent();
		}
	}
	playerGotTarget(nextStep, plyr, target);
	return nextStep;
}

Point2D GeneralAStar(Point2D target, Point2D player, Character *plyr)
{
	if (player == target)
	{
		return target;
	}
	Point2D nextStep;
	int min = MAXINT;
	list<Node> nodes;
	vector<Node> visited;
	vector<Node>::iterator visited_it;
	Node nd = *new Node(player, target, 0), current;
	clearQue();
	bestNodes2.push(nd);
	bool finished = false;
	path.push_back(*new Parent(nd.GetPoint(), nd.GetPoint(), false));
	while (!bestNodes2.empty() && !finished)
	{
		current = bestNodes2.top();
		bestNodes2.pop();

		//if reached target finish the route search
		if (current.GetPoint() == target)
		{
			nodes.push_back(current);
			finished = true;
		}
		//otherwise keep searching
		else
		{
			visited_it = find(visited.begin(), visited.end(), current);
			if (visited_it == visited.end())
				visited.push_back(current);
			else
				continue;
			int dx, dy;
			for (int i = 0; i < 4; i++)
			{
				dx = (i % 2) * (2 * (i / 2) - 1);
				dy = (1 - i % 2)* (2 * (i / 2) - 1);
				if (maze[current.GetPoint().GetY() + dy][current.GetPoint().GetX() + dx] != WALL)
				{
					Point2D pt = *new Point2D(current.GetPoint().GetX() + dx, current.GetPoint().GetY() + dy);
					Node nd = *new Node(pt, target, current.GetG() + 1);
					path.push_back(*new Parent(pt, current.GetPoint(), true));
					bestNodes2.push(nd);
				}
			}
		}
		if (current.GetPoint() == target)
		{
			Parent targetPoint;
			list<Parent>::iterator iter = path.end();
			for (iter--; iter != path.begin(); --iter)
			{
				targetPoint = *iter;
				if (targetPoint.HasParent() && targetPoint.GetCurrent() == current.GetPoint())
				{
					if (targetPoint.GetPrev() == player)
						break;
					current = *new Node(targetPoint.GetPrev(), target, 0);
				}
			}
			nextStep = targetPoint.GetCurrent();
		}
	}
	playerGotTarget(nextStep, plyr, target);
	return nextStep;
}

Point2D AStarForEscape(Point2D player, Character *plyr1, Character *plyr2, Point2D* lastExit, Point2D lastExitOfRival, Point2D rival, bool *middle)
{
	Point2D nextStep;
	Point2D target;
	int roomNumber1 = plyr1->GetCurrRoom();
	int roomNumber2 = plyr2->GetCurrRoom();
	if (roomNumber1 != -1 && !(*middle))
	{
		target = TargetOneRoomAway(all_rooms[roomNumber1], roomNumber2, player, lastExitOfRival);
		*lastExit = target;
		nextStep = CarefulAStar(target, player, plyr1, rival);
		if (maze[nextStep.GetY()][nextStep.GetX()] == TUNNEL)
		{
			plyr1->SetCurrRoom(-1);
		}
		return nextStep;
	}
	else if (*middle)
	{
		target = all_rooms[roomNumber1].GetCenter();
		nextStep = CarefulAStar(target, player, plyr1, rival);
		if (target == nextStep)
		{
			plyr1->SetChoice(-1);
			*middle = false;
		}
		return nextStep;
	}
	else
	{
		target = getExitPoint(*lastExit);
		nextStep = GeneralAStar(target, player, plyr1);
		if (target == nextStep)
		{
			plyr1->SetCurrRoom(GetRoomNumberOfExit(nextStep));
			(*middle) = true;
		}
		return nextStep;
	}
	
}

Point2D AStarForPlayer(Point2D hunter, Point2D hunted, Character *plyr1, Character *plyr2, Point2D* lastExitOfHunter , Point2D* lastExitOfHunted)
{
	Point2D nextStep;
	int roomNumber1 = plyr1->GetCurrRoom();
	int roomNumber2 = plyr2->GetCurrRoom();
	Point2D target;
	if (roomNumber1 == roomNumber2 && roomNumber1 != -1)
	{
		return GeneralAStar(hunted, hunter, plyr1);
	}
	else if (roomNumber2 != -1 && roomNumber1 != -1 && roomNumber1 != roomNumber2)
	{

		target = BFSOnRoomForHunted(all_rooms[roomNumber1], roomNumber1,roomNumber2, hunter, getExitPoint(*lastExitOfHunted), *lastExitOfHunted);
		*lastExitOfHunter = target;
		nextStep = GeneralAStar(target, hunter, plyr1);
		if (maze[nextStep.GetY()][nextStep.GetX()] == TUNNEL)
		{
			plyr1->SetCurrRoom(-1);
		}
		return nextStep;
	}
	else if (roomNumber2 == -1 && roomNumber1 != -1)
	{
		
		target = BFSOnRoomForHunted(all_rooms[roomNumber1], roomNumber1, GetRoomNumberOfExit(getExitPoint(*lastExitOfHunted)), hunter, getExitPoint(*lastExitOfHunted), *lastExitOfHunted);
		*lastExitOfHunter = target;
		nextStep = GeneralAStar(target, hunter, plyr1);
		if (maze[nextStep.GetY()][nextStep.GetX()] == TUNNEL)
		{
			plyr1->SetCurrRoom(-1);
		}
		return nextStep;
	}
	else 
	{
		target = getExitPoint(*lastExitOfHunter);
		nextStep = GeneralAStar(target, hunter, plyr1);
		if (target == nextStep)
		{
			plyr1->SetCurrRoom(GetRoomNumberOfExit(nextStep));
		}
		return nextStep;
	}
}

Point2D AStarForRoomSearch(const int LABEL, Point2D player, Character *plyr, Point2D* lastExit, const int NPC, Point2D lastExitOfRival, Character *plyr2, Point2D rival)
{
	Point2D nextStep;
	int roomNumber = plyr->GetCurrRoom();
	Point2D target;
	int newLabel = LABEL;
	//player not in tunnel
	if (roomNumber != -1)
	{
		do
		{
			target = BFSOnRooms(all_rooms[roomNumber], newLabel, player, lastExitOfRival,plyr2->GetCurrRoom());
			if (target == bad)
			{
				if (NPC == NPC1)
				{
					plyr->SetChoice(selectNewRandomTargetChoice(*plyr, npc2));
				}
				else
				{
					plyr->SetChoice(selectNewRandomTargetChoice(*plyr, npc1));
				}

				newLabel = getLabelFromChoice(plyr->GetChoice());
				if (newLabel == -1)
				{
					return player;
				}
			}
		} while (target  == bad);
		
		if (maze[target.GetY()][target.GetX()] == TUNNEL || 
			maze[target.GetY()][target.GetX()] == NPC)
		{
			*lastExit = target;
		}
		nextStep = CarefulAStar(target, player, plyr,rival);
		if (maze[nextStep.GetY()][nextStep.GetX()] == TUNNEL
			|| maze[nextStep.GetY()][nextStep.GetX()] == NPC)
		{
			plyr->SetCurrRoom(-1);
		}
		return nextStep;
	}
	else
	{
		target = getExitPoint(*lastExit);
		nextStep = GeneralAStar(target, player, plyr);
		if (target == nextStep)
		{
			plyr->SetCurrRoom(GetRoomNumberOfExit(nextStep));
		}
		return nextStep;
	}
}

void SetupMaze()
{
	int i, j,counter;
	int left, right, top, bottom;
	bool isValidRoom;
	Room* pr=NULL;

	for (counter = 0; counter < NUM_ROOMS; counter++)
	{
		// create room
		do
		{
			free(pr);
			pr = new Room(Point2D(rand()%MSIZE,
			rand() % MSIZE), 5 + rand() % 15, 5 + rand() % 25);
			top = pr->GetCenter().GetY() - pr->GetHeight() / 2;
			if (top < 0) top = 0;
			bottom = pr->GetCenter().GetY() + pr->GetHeight() / 2;
			if (bottom >= MSIZE) bottom = MSIZE - 1;
			left = pr->GetCenter().GetX() - pr->GetWidth() / 2;
			if (left < 0) left = 0;
			right = pr->GetCenter().GetX() + pr->GetWidth() / 2;
			if (right >= MSIZE) right = MSIZE - 1;

			isValidRoom = true;
			for (i = 0; i < counter && isValidRoom; i++)
				if (all_rooms[i].IsOverlap(*pr))
					isValidRoom = false;

		} while (!isValidRoom);

		all_rooms[counter] = *pr;
		for (i = top; i <= bottom; i++)
			for (j = left; j <= right; j++)
				maze[i][j] = SPACE;

	}
	DigTunnels();
	RecordExits();
}

void DrawMaze()
{
	int i, j;

	for(i = 0;i<MSIZE;i++)
		for (j = 0; j < MSIZE; j++)
		{
			switch (maze[i][j])
			{
			case WALL:
				glColor3d(0.4, 0, 0); // dark red;
				break;
			case SPACE:
				glColor3d(1, 1, 1); // white;
				break;
			case TUNNEL:
				glColor3d(1, 0, 1); // white;
				break;
			case NPC1:
				glColor3d(0, 0, 1); // Blue;
				break;
			case NPC2:
				glColor3d(1, 0, 0); // Red;
				break;
			case AMMO:
				glColor3d(0, 0, 0); // Black;
				break;
			case HEALTH:
				glColor3d(0, 1, 0); // Green;
				break;
			case DEAD:
				glColor3d(0, 0.3, 0); // white;
				break;

			}
			// draw square
			glBegin(GL_POLYGON);
				glVertex2d(j*SQSIZE - 1- SQSIZE/2, i*SQSIZE - 1+SQSIZE/2);
				glVertex2d(j*SQSIZE - 1 + SQSIZE / 2, i*SQSIZE - 1 + SQSIZE / 2);
				glVertex2d(j*SQSIZE - 1 + SQSIZE / 2, i*SQSIZE - 1 - SQSIZE / 2);
				glVertex2d(j*SQSIZE - 1 - SQSIZE / 2, i*SQSIZE - 1 - SQSIZE / 2);
			glEnd();
		}

}


int getNumberOfRoom(Point2D point)
{
	for (int roomIndex = 0; roomIndex < NUM_ROOMS; roomIndex++)
	{
		int top, bottom, left, right;
		top = all_rooms[roomIndex].GetCenter().GetY() - all_rooms[roomIndex].GetHeight() / 2;
		bottom = all_rooms[roomIndex].GetCenter().GetY() + all_rooms[roomIndex].GetHeight() / 2;
		left = all_rooms[roomIndex].GetCenter().GetX() - all_rooms[roomIndex].GetWidth() / 2;
		right = all_rooms[roomIndex].GetCenter().GetX() + all_rooms[roomIndex].GetWidth() / 2;

		if (point.GetX() <= right && point.GetX() >= left && point.GetY() >= top && point.GetY() <= bottom)
			return roomIndex;
	}
	return -1;
}

void SetNpcStartLocation()
{
	do
	{
		player1 = *new Point2D(rand() % MSIZE, rand() % MSIZE);
	} while (maze[player1.GetY()][player1.GetX()] != SPACE);

	maze[player1.GetY()][player1.GetX()] = NPC1;
	npc1 = *new Character(STARTHEALTH, STARTBULLETS, -1, getNumberOfRoom(player1));
	nextPoint1 = NPC1;

	do
	{
		player2 = *new Point2D(rand() % MSIZE, rand() % MSIZE);
	} while (maze[player2.GetY()][player2.GetX()] != SPACE);

	npc2 = *new Character(STARTHEALTH, STARTBULLETS, -1, getNumberOfRoom(player2));
	maze[player2.GetY()][player2.GetX()] = NPC2;
	nextPoint2 = NPC2;

	middle1 = false;
	middle2 = false;

}

void SetArmorAndHelathLocation()
{
	int top, bottom, right, left;
	Point2D weapon, health;
	int i, j;
	for (i = 0; i < NUM_ROOMS; i++)
	{

		top = all_rooms[i].GetCenter().GetY() - all_rooms[i].GetHeight() / 2;
		if (top < 0) top = 0;
		bottom = all_rooms[i].GetCenter().GetY() + all_rooms[i].GetHeight() / 2;
		if (bottom >= MSIZE) bottom = MSIZE - 1;
		left = all_rooms[i].GetCenter().GetX() - all_rooms[i].GetWidth() / 2;
		if (left < 0) left = 0;
		right = all_rooms[i].GetCenter().GetX() + all_rooms[i].GetWidth() / 2;
		if (right >= MSIZE) right = MSIZE - 1;
		for (j = 0; j < NUMOFWEAPONANDHEALTHINROOM; j++)
		{
			do
			{
				weapon = *new Point2D(rand() % (bottom - top) + top,
					rand() % (right - left) + left);
			} while (maze[weapon.GetX()][weapon.GetY()] != SPACE);
			do
			{
				health = *new Point2D(rand() % (bottom - top) + top,
					rand() % (right - left) + left);
			} while (maze[health.GetX()][health.GetY()] != SPACE);
			maze[weapon.GetX()][weapon.GetY()] = AMMO;
			maze[health.GetX()][health.GetY()] = HEALTH;

		}
	}
}

void Game()
{
	Point2D nextStep1, nextStep2;
	int y, x;
	if (npc2.GetHealth() <= 0)
	{
		game = false;
		printf("Game Over, Player 1 Wins");
		maze[player2.GetY()][player2.GetX()] = DEAD;
	}
	else if (npc1.GetHealth() <= 0)
	{
		game = false;
		printf("Game Over, Player 2 Wins");
		maze[player1.GetY()][player1.GetX()] = DEAD;
	}
	else
	{
		if (npc1.GetChoice() == -1)
		{
			int x = selectNewRandomTargetChoice(npc1, npc2);
			npc1.SetChoice(x);
		}

		if (npc2.GetChoice() == -1)
		{
			int x = selectNewRandomTargetChoice(npc2, npc1);
			npc2.SetChoice(x);
		}

		switch (npc1.GetChoice())
		{
			// player 1 searches for health
		case 1:
			x = player1.GetX();
			y = player1.GetY();
			
			nextStep1 = AStarForRoomSearch(HEALTH, player1, &npc1, lastExit1, NPC1, *lastExit2, &npc2, player2);
			if (!(nextStep1 == player2))
			{
				player1 = nextStep1;

				if (nextPoint1 == TUNNEL)
				{
					maze[y][x] = TUNNEL;
				}
				else
				{
					maze[y][x] = SPACE;
				}
				nextPoint1 = maze[player1.GetY()][player1.GetX()];
				maze[player1.GetY()][player1.GetX()] = NPC1;
			}
			break;
			// player 1 searches for ammo
		case 2:
			x = player1.GetX();
			y = player1.GetY();
			
			nextStep1 = AStarForRoomSearch(AMMO, player1, &npc1, lastExit1, NPC1,*lastExit2, &npc2,player2);
			if (!(nextStep1 == player2))
			{
				player1 = nextStep1;
				if (nextPoint1 == TUNNEL)
				{
					maze[y][x] = TUNNEL;
				}
				else
				{
					maze[y][x] = SPACE;
				}
				nextPoint1 = maze[player1.GetY()][player1.GetX()];
				maze[player1.GetY()][player1.GetX()] = NPC1;
			}
			break;
		case 3:
			x = player1.GetX();
			y = player1.GetY();
			nextStep1 = AStarForEscape(player1, &npc1, &npc2, lastExit1, *lastExit2, player2,&middle1);
			if (!(nextStep1 == player2))
			{
				player1 = nextStep1;
				if (nextPoint1 == TUNNEL)
				{
					maze[y][x] = TUNNEL;
				}
				else
				{
					maze[y][x] = SPACE;
				}
				nextPoint1 = maze[player1.GetY()][player1.GetX()];
				maze[player1.GetY()][player1.GetX()] = NPC1;
			}
			break;
		case 0:
			x = player1.GetX();
			y = player1.GetY();
			int top = y - 3;
			if (top < 0) top = 0;
			int bottom = y + 3;
			if (bottom > MSIZE) bottom = MSIZE;
			int left = x - 3;
			if (left < 0) left = 0;
			int right = x + 3;
			if (right > MSIZE) right = MSIZE;
			bool stop = false;
			for (int i = top; i < bottom && !stop; i++)
			{
				for (int j = left; j < right; j++)
				{
					if (maze[i][j] == NPC2)
					{
						npc2.SetHealth(npc2.GetHealth() - 4 + max(abs(i - y), abs(j - x)));
						stop = true;
						npc1.SetAmmo(npc1.GetAmmo() - 1);
						npc1.SetChoice(-1);
						printf("Player1 shot Player2 for %d damage: Player2 at %d HP\n", 4 - max(abs(i - y), abs(j - x)),npc2.GetHealth());
						break;
					}
				}
			}
			if (!stop)
			{
				nextStep1 = AStarForPlayer(player1, player2, &npc1, &npc2, lastExit1, lastExit2);
				if (!(nextStep1 == player2))
				{
					player1 = nextStep1;
					if (nextPoint1 == TUNNEL)
					{
						maze[y][x] = TUNNEL;
					}
					else
					{
						maze[y][x] = SPACE;
					}
					nextPoint1 = maze[player1.GetY()][player1.GetX()];
					maze[player1.GetY()][player1.GetX()] = NPC1;
				}
			}
			
			break;
		}

		switch (npc2.GetChoice())
		{
			// player 2 searches for health
		case 1:
			x = player2.GetX();
			y = player2.GetY();

			nextStep2 = AStarForRoomSearch(HEALTH, player2, &npc2, lastExit2, NPC2, *lastExit1, &npc1, player1);
			if (!(nextStep2 == player1))
			{
				player2 = nextStep2;
				if (nextPoint2 == TUNNEL)
				{
					maze[y][x] = TUNNEL;
				}
				else
				{
					maze[y][x] = SPACE;
				}
				if (!(player1 == player2))
				{
					nextPoint2 = maze[player2.GetY()][player2.GetX()];
				}
				maze[player2.GetY()][player2.GetX()] = NPC2;
			}
			break;
			// player 2 searches for ammo
		case 2:
			x = player2.GetX();
			y = player2.GetY();

			nextStep2 = AStarForRoomSearch(AMMO, player2, &npc2, lastExit2, NPC2, *lastExit1, &npc1, player1);
			if (!(nextStep2 == player1))
			{
				player2 = nextStep2;
				if (nextPoint2 == TUNNEL)
				{
					maze[y][x] = TUNNEL;
				}
				else
				{
					maze[y][x] = SPACE;
				}
				if (!(player1 == player2))
				{
					nextPoint2 = maze[player2.GetY()][player2.GetX()];
				}
				maze[player2.GetY()][player2.GetX()] = NPC2;
			}
			break;
		case 3:
			x = player2.GetX();
			y = player2.GetY();
			nextStep2 = AStarForEscape(player2, &npc2, &npc1, lastExit2, *lastExit1, player1,&middle2);
			if (!(nextStep2 == player1))
			{
				player2 = nextStep2;
				if (nextPoint2 == TUNNEL)
				{
					maze[y][x] = TUNNEL;
				}
				else
				{
					maze[y][x] = SPACE;
				}
				if (!(player1 == player2))
				{
					nextPoint2 = maze[player2.GetY()][player2.GetX()];
				}
				maze[player2.GetY()][player2.GetX()] = NPC2;
			}
			break;
		case 0:
			x = player2.GetX();
			y = player2.GetY();
			int top = y - 3;
			if (top < 0) top = 0;
			int bottom = y + 3;
			if (bottom > MSIZE) bottom = MSIZE;
			int left = x - 3;
			if (left < 0) left = 0;
			int right = x + 3;
			if (right > MSIZE) right = MSIZE;
			bool stop = false;
			for (int i = top; i < bottom && !stop; i++)
			{
				for (int j = left; j < right; j++)
				{
					if (maze[i][j] == NPC1)
					{
						npc1.SetHealth(npc1.GetHealth() - 4 + max(abs(i - y), abs(j - x)));
						stop = true;
						npc2.SetAmmo(npc2.GetAmmo() - 1);
						npc2.SetChoice(-1);
						printf("Player2 shot Player1 for %d damage: Player1 at %d HP\n", 4 - max(abs(i - y), abs(j - x)), npc1.GetHealth());
						break;
					}
				}
			}
			if (!stop)
			{
				nextStep2 = AStarForPlayer(player2, player1, &npc2, &npc1, lastExit2, lastExit1);
				if (!(nextStep2 == player1))
				{
					player2 = nextStep2;
					if (nextPoint2 == TUNNEL)
					{
						maze[y][x] = TUNNEL;
					}
					else
					{
						maze[y][x] = SPACE;
					}
					if (!(player1 == player2))
					{
						nextPoint2 = maze[player2.GetY()][player2.GetX()];
					}
					maze[player2.GetY()][player2.GetX()] = NPC2;
				}
			}
			break;
		}
		if (nextStep1 == player2 && nextStep2 == player1)
		{
			player1 = nextStep1;
			player2 = nextStep2;
			int tmp = nextPoint1;
			nextPoint1 = nextPoint2;
			nextPoint2 = tmp;
			maze[player1.GetY()][player1.GetX()] = NPC1;
			maze[player2.GetY()][player2.GetX()] = NPC2;
		}
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	DrawMaze();

	glutSwapBuffers();// show what was drawn in "frame buffer"
}

void idle()
{
	if (game) Game();
	glutPostRedisplay();// calls indirectly to display
}

void Menu(int choice)
{
	switch (choice)
	{
	case 1:
		game = true;
		break;
	case 2:
		game = false;
		init();
		break;
	}
}

void main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("Dungeoun Project ");

	glutDisplayFunc(display); // refresh function
	glutIdleFunc(idle); // idle: when nothing happens
	init();

	glutCreateMenu(Menu);
	glutAddMenuEntry("Start Game", 1);
	glutAddMenuEntry("Restart", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);


	glutMainLoop();
}