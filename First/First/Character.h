#pragma once
class Character
{
public:
	Character();
	~Character();
private:

	int Health;
	int Ammo;
	int Choice;
	int CurrRoom;
public:
	Character(int Health, int Bullets, int Choice, int CurrRoom);
	int GetHealth() const;
	int GetAmmo() const;
	void SetAmmo(int A);
	void SetHealth(int H);
	int GetChoice() const;
	void SetChoice(int C);
	int GetCurrRoom() const;
	void SetCurrRoom(int C);
	bool operator==(const Character& other);
}; 
