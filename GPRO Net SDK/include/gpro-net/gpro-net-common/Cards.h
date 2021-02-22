#ifndef CARDS_H
#define CARDS_H


const int NUM_DECK_CARDS = 52;
const int NUM_SUITE_CARDS = 13;

typedef unsigned short CardData;


typedef struct _Card
{
	CardData suit;
	CardData value;
}Card;

Card drawCard();


#endif // !CARDS_H