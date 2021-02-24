#include "gpro-net/gpro-net-common/CardBST.h"

// Binary Search Tree

//typedef struct _Node
//{
//	Card data;
//	struct node* rightChild;
//	struct node* leftChild;
//}Node;
//
//Node* search(Node* root, Card x)
//{
//	if (root == NULL || (root->data.suit == x.suit && root->data.value == x.value))
//		return root;
//	else if (x.value < root->data.value)
//		return search(root->leftChild, x);
//	else if (x.value == root->data.value)
//	{
//		if (x.suit <= root->data.suit)
//			return search(root->leftChild, x);
//		else
//			return search(root->rightChild, x);
//	}
//	else
//		return search(root->rightChild, x);
//}
//
//Node* createNode(Card x)
//{
//	Node* newNode;
//	newNode = malloc(sizeof(Node));
//	newNode->data = x;
//	newNode->leftChild = NULL;
//	newNode->rightChild = NULL;
//
//	return newNode;
//}
//
//Node* insert(Node* root, Card newCard)
//{
//
//}
