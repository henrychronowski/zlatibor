/*
   Copyright 2021 Daniel S. Buckstein

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	GPRO Net SDK: Networking framework.
	By Daniel S. Buckstein

	gpro-net.c
	Main source for framework.
*/

#include "gpro-net/gpro-net.h"
#include <stdlib.h>
#include <time.h>

char randSeeded = '0';

// Seeds the pseudo random number generator once
void seedRand()
{
	if (randSeeded == '0')
	{
		srand((unsigned)time(NULL));
		randSeeded = '1';
	}
}

Card drawCard()
{
	seedRand();

	int randCard = (rand() % NUM_DECK_CARDS) + 1;

	Card card;
	card.suit = (CardData)(randCard / NUM_SUITE_CARDS);
	card.value = (CardData)(randCard % NUM_SUITE_CARDS) + 1;

	if (card.value == 1)
		card.value = 14;

	return card;
}

void printCard(Card card)
{
	char cardSuit = ' ';
	switch (card.suit)
	{
	case 0:
		cardSuit = 'S';
		break;
	case 1:
		cardSuit = 'C';
		break;
	case 2:
		cardSuit = 'H';
		break;
	case 3:
		cardSuit = 'D';
		break;
	default:
		cardSuit = (char)card.suit;
		break;
	}

	char cardValue = ' ';
	switch (card.value)
	{
	case 11:
		cardValue = 'J';
		printf("%c, %c | ", cardSuit, cardValue);
		break;
	case 12:
		cardValue = 'Q';
		printf("%c, %c | ", cardSuit, cardValue);
		break;
	case 13:
		cardValue = 'K';
		printf("%c, %c | ", cardSuit, cardValue);
		break;
	case 1:
	case 14:
		cardValue = 'A';
		printf("%c, %c | ", cardSuit, cardValue);
		break;
	default:
		printf("%c, %u | ",  cardSuit, card.value);
		break;
	}
}