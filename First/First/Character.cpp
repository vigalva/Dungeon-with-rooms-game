#include "Character.h"

Character::Character()
{
}

Character::~Character()
{
}

Character::Character(int Health, int Ammo, int Choice, int CurrRoom)
{
	this->Health = Health;

	this->Ammo = Ammo;

	this->Choice = Choice;

	this->CurrRoom = CurrRoom;
}

int Character::GetHealth() const
{
	return Health;
}

int Character::GetAmmo() const
{
	return Ammo;
}



void Character::SetAmmo(int A)
{
	Ammo = A;

}

void Character::SetHealth(int H)
{
	Health = H;
}

int Character::GetChoice() const
{
	return Choice;
}

void Character::SetChoice(int C)
{
	Choice = C;
}

int Character::GetCurrRoom() const
{
	return CurrRoom;
}

void Character::SetCurrRoom(int C)
{
	CurrRoom = C;
}

bool Character::operator==(const Character & other)
{
	return Health == other.Health && Ammo == other.Ammo && Choice == other.Choice 
		&& CurrRoom == other.CurrRoom;
}

