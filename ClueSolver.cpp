#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
using namespace std;

class Player {
public:
	string name;
	int numCards;
	unordered_set<string> knownCards;
	unordered_set<string> possibleCards;
	vector<vector<string> > priorMoves;
	/////change this!!!!!
	unordered_map<string,double> probs;
};

vector<string> allCards {"SCARLET", "GREEN", "MUSTARD", "PLUM", "PEACOCK", "WHITE", "CANDLESTICK", "KNIFE", "PIPE", "REVOLVER", 
"ROPE", "WRENCH", "HALL", "LOUNGE", "DINING", "KITCHEN", "BALL", "CONSERVATORY", "BILLIARDS", "LIBRARY", "STUDY"};

struct Confidential {
	unordered_set<string> Suspects {"SCARLET", "GREEN", "MUSTARD", "PLUM", "PEACOCK", "WHITE"};
	unordered_set<string> Weapons {"CANDLESTICK", "KNIFE", "PIPE", "REVOLVER", "ROPE", "WRENCH"};
	unordered_set<string> Rooms {"HALL", "LOUNGE", "DINING", "KITCHEN", "BALL", "CONSERVATORY", "BILLIARDS", "LIBRARY", "STUDY"};
};

void fillPossibleCardsInit(vector<Player> & players, Confidential conf);
void getFirstPlayer(int & playingPlayerIndex, int numPlayers, vector<Player> & players);
void check_possible_lists(vector<Player> & players, Confidential & confidential);
void runTurn(vector<Player> & players, int playingPlayerIndex, Confidential & conf);
void cardShown(Confidential & confidential, string card, vector<Player> & players, int personWithCardIndex);
void printProbabilities(vector<string> cardList, vector<Player> players);
void runCascadingInfoChecks(vector<Player> & players, Confidential & conf);
bool runDidShowCheck(vector<string> newTurnTrio, int responderIndex, vector<Player> & players, Confidential & conf);
void printHands(vector<Player> players, Confidential conf);
void checkForGG(Confidential conf);

int main() {

	
	//Prompts for number of players, player names, number of cards ////////

	Confidential confidential;
				
	cout << "Please enter number of players:" << endl;

	string firstPlayer;

	int numPlayers = 0;

	int playingPlayerIndex = -1;

	vector<Player> players;

	cin >> numPlayers;

	cout << "Please enter player names and number of cards:" << endl;

	for (int i=0;i<numPlayers; ++i) {
		Player p;
		cout << "Name:" << endl;
		cin >> p.name;
		cout << "Number of cards:" << endl;
		cin >> p.numCards;
		players.push_back(p);
		for (int j=0;j<allCards.size();++j) {
			players[i].probs[allCards[j]] = ((double)p.numCards / (double)allCards.size());	
		}
	}
	
	/////////////////////////////////////////////////////////////////////////

	///////   Determining who's up /////////////////////////////////////

	getFirstPlayer(playingPlayerIndex, numPlayers, players);

	////////////// Logging info about user's player /////////////////////

	cout << "Enter your cards:" << endl;

	//set player's known cards to 1
	for (int i=0;i<players[0].numCards;++i) {
		string card;
		cin >> card;
		players[0].knownCards.insert(card);
		players[0].probs[card] = 1;
	}

	//set all other of player's cards to 0
	for (int i=0;i<allCards.size();++i) {
		if (players[0].probs[allCards[i]] != 1) {
			players[0].probs[allCards[i]] = 0;
		}
	}

	fillPossibleCardsInit(players,confidential);

	//printProbabilities(allCards,players);

	for (unordered_set<string>::iterator it = players[0].knownCards.begin(); it != players[0].knownCards.end(); ++it) {
		cardShown(confidential, *it, players, 0);
	}
	//////////////////////////////////////////////////////////////////////

	while (cin) {
		//if (cin.eof()) break;
		//printHands(players, confidential);
		runTurn(players,playingPlayerIndex,confidential);
		check_possible_lists(players,confidential);
		checkForGG(confidential);
		++playingPlayerIndex;
		playingPlayerIndex = playingPlayerIndex % players.size();
	}

	return 0;
}

void fillPossibleCardsInit(vector<Player> & players, Confidential conf) {

	for (int i=1;i<players.size();++i) {

		for (unordered_set<string>::iterator it = conf.Suspects.begin(); it != conf.Suspects.end(); ++it) {
			players[i].possibleCards.insert(*it); 
		}
		for (unordered_set<string>::iterator it = conf.Weapons.begin(); it != conf.Weapons.end(); ++it) {
			players[i].possibleCards.insert(*it); 
		}
		for (unordered_set<string>::iterator it = conf.Rooms.begin(); it != conf.Rooms.end(); ++it) {
			players[i].possibleCards.insert(*it); 
		}

		for (int j=0;j<allCards.size();++j) {
			if (players[0].probs[allCards[j]] == 1) {
				players[i].probs[allCards[j]] = 0;
			}
			else {
				players[i].probs[allCards[j]] = ((double)players[i].numCards / (double)(allCards.size() - players[0].numCards)); 
			}
		}
	}

}

void runTurn(vector<Player> & players, int playingPlayerIndex, Confidential & conf) {
	string suspect;
	string weapon;
	string room;

	cout << "Player " << players[playingPlayerIndex].name << " guesses (if no guess enter PASS): " << endl;

	cout << "Suspect: " << endl;

	cin >> suspect;

	if (suspect == "PASS") {
		return;
	}

	cout << "Weapon: " << endl;

	cin >> weapon;

	cout << "Room: " << endl;

	cin >> room;

	cout << "Playing Player Index: " << playingPlayerIndex << endl;

	for (int i=1; i<players.size();++i) {
		string didShowStr;
		int responderIndex;
		responderIndex = (i + playingPlayerIndex) % players.size();
		if (responderIndex == playingPlayerIndex) {
			return;
		}
		cout << "Did " << players[responderIndex].name << " show? (y or n)" << endl;
		cin >> didShowStr;
		cout << "Responder Index: " << responderIndex << endl;
		if ((didShowStr == "y") /*&& (responderIndex != 1)*/) {
			
			if (playingPlayerIndex == 0) {
				string cardOwned;
				cout << "What card did they show?" << endl;
				cin >> cardOwned;
				cardShown(conf, cardOwned, players, responderIndex);
				runCascadingInfoChecks(players,conf);
			}
			else {
				vector<string> newTurnTrio {weapon,suspect,room};
				players[responderIndex].priorMoves.push_back(newTurnTrio);
				cout << players[responderIndex].priorMoves[0][0] << endl;
				if (runDidShowCheck(newTurnTrio,responderIndex,players,conf)) {
					runCascadingInfoChecks(players,conf);
				}
			}

			return;
		}
		if (didShowStr == "n") {
			players[responderIndex].possibleCards.erase(weapon);
			players[responderIndex].possibleCards.erase(suspect);
			players[responderIndex].possibleCards.erase(room);

			runCascadingInfoChecks(players, conf);
		}

	}
}

void check_possible_lists(vector<Player> & players, Confidential & confidential) {
	unordered_set<string> leftoverRooms = confidential.Rooms;
	unordered_set<string> leftoverWeapons = confidential.Weapons;
	unordered_set<string> leftoverSuspects = confidential.Suspects;

	for (int i=0;i<players.size();++i) {

		for (unordered_set<string>::iterator it_play = players[i].possibleCards.begin(); it_play != players[i].possibleCards.end(); ++it_play) {
			if (leftoverSuspects.find(*it_play) != leftoverSuspects.end()) {
				 leftoverSuspects.erase(leftoverSuspects.find(*it_play));
			}
			if (leftoverWeapons.find(*it_play) != leftoverWeapons.end()) {
				 leftoverWeapons.erase(leftoverWeapons.find(*it_play));
			}
			if (leftoverRooms.find(*it_play) != leftoverRooms.end()) {
				 leftoverRooms.erase(leftoverRooms.find(*it_play));
			}
		}
	}

	if (leftoverSuspects.size() > 0) {
		confidential.Suspects = leftoverSuspects;
	}
	if (leftoverWeapons.size() > 0) {
		confidential.Weapons = leftoverWeapons;
	}
	if (leftoverRooms.size() > 0) {
		confidential.Rooms = leftoverRooms;
	}
}

void checkForGG(Confidential conf) {
	if (conf.Rooms.size() == 1) {
		cout << "Found a Room!!!!: " << *(conf.Rooms.begin()) << endl;
	}
	if (conf.Suspects.size() == 1) {
		cout << "Found a Suspect!!!!: " << *(conf.Suspects.begin()) << endl;
	}
	if (conf.Weapons.size() == 1) {
		cout << "Found a Weapon!!!!: " << *(conf.Weapons.begin()) << endl;
	}
}

bool runDidShowCheck(vector<string> newTurnTrio, int responderIndex, vector<Player> & players, Confidential & conf) {

	int possibleCardsCount = 0;

	string isolatedCard;

	if (((players[responderIndex].numCards - players[responderIndex].knownCards.size()) == players[responderIndex].possibleCards.size()) && (players[responderIndex].possibleCards.size() > 0)) {
		for (unordered_set<string>::iterator it = players[responderIndex].possibleCards.begin(); it != players[responderIndex].possibleCards.end(); ++it) {
			cardShown(conf,*it,players,responderIndex);
		}
		return true;
	}

	for (int i=0;i<newTurnTrio.size();++i) {
		unordered_set<string>::const_iterator foundPossibleCard = players[responderIndex].possibleCards.find(newTurnTrio[i]);
		unordered_set<string>::const_iterator foundKnownCard = players[responderIndex].knownCards.find(newTurnTrio[i]);
		if ((foundPossibleCard != players[responderIndex].possibleCards.end()) || (foundKnownCard != players[responderIndex].knownCards.end())) {
			++possibleCardsCount;
			isolatedCard = newTurnTrio[i];
		}
	}

	if ((possibleCardsCount == 1) && (players[responderIndex].possibleCards.find(isolatedCard) != players[responderIndex].knownCards.end())) {
		cardShown(conf,isolatedCard,players,responderIndex);
		/*players[responderIndex].knownCards.insert(isolatedCard);
		for (int i=0;i<players.size();++i) {
			players[i].possibleCards.erase(isolatedCard);
		}*/
		return true;
	}
	else {
		return false;
	}

}


void runCascadingInfoChecks(vector<Player> & players, Confidential & conf) {

	bool infoGained = true;
	while (infoGained) {
		infoGained = false;
		for (int i=0; i<players.size();++i) {
			for (int j=0;j<players[i].priorMoves.size();++j) {
				if (runDidShowCheck(players[i].priorMoves[j],i,players,conf)) {
					infoGained = true;
				}
			} 
		}
	}
}


void getFirstPlayer(int & playingPlayerIndex, int numPlayers, vector<Player> & players) {

	string firstPlayer;

	while (1) {

		cout << "Who is going first?" << endl;

		cin >> firstPlayer;	

		for (int i=0;i<numPlayers;++i) {
			if (players[i].name == firstPlayer) {
				playingPlayerIndex = i;
			} 
		}

		if (playingPlayerIndex == -1) {
			cout << "Not a valid player name" << endl;
		}
		else {
			break;
		}
	}
}

void cardShown(Confidential & confidential, string card, vector<Player> & players, int personWithCardIndex) {

	for (int i=0;i<players.size();++i) {
		players[i].possibleCards.erase(card);
	}

	players[personWithCardIndex].knownCards.insert(card);

	if (players[personWithCardIndex].knownCards.size() == players[personWithCardIndex].numCards) {
		players[personWithCardIndex].possibleCards.clear();
	}

	if (confidential.Suspects.find(card) != confidential.Suspects.end()) {
		confidential.Suspects.erase(confidential.Suspects.find(card));
	}
	
	if (confidential.Weapons.find(card) != confidential.Weapons.end()) {
		confidential.Weapons.erase(confidential.Weapons.find(card));
	}

	if (confidential.Rooms.find(card) != confidential.Rooms.end()) {
		confidential.Rooms.erase(confidential.Rooms.find(card));
	}
}

void printProbabilities(vector<string> cardList, vector<Player> players) {

	for (int i=0; i<players.size(); ++i) {
		cout << players[i].name << "'s probabilities of having cards" << endl;
		for (int j=0; j<cardList.size();++j) {
			cout << cardList[j] << ": " << players[i].probs[cardList[j]] << " ";
		}
		cout << endl;
	}

}


void printHands(vector<Player> players, Confidential conf) {


	cout << "-------------PRINT HANDS CALLED: INFO BELOW!!! -------------" << endl;

	for (int i=0;i<players.size();++i) {
		cout << players[i].name << "'s Hand:" << endl;
		cout << "Known Cards: ";
		for (unordered_set<string>::iterator it = players[i].knownCards.begin(); it != players[i].knownCards.end(); ++it) {
			cout <<  *it << " ";
		}
		cout << endl;
		cout << "Possible Cards: ";
		for (unordered_set<string>::iterator it = players[i].possibleCards.begin(); it != players[i].possibleCards.end(); ++it) {
			cout << *it << " ";
		}
		cout << endl;
		cout << "Past Moves: " << endl;
		for (int j=0; j<players[i].priorMoves.size();++j) {
			cout << "Move: " << j << endl;
			for (int k=0;k<players[i].priorMoves[j].size();++k) {
				 cout << players[i].priorMoves[j][k] << " ";
			}
			cout << endl;
		}
	}

	cout << "Confidential Info:" << endl;
	
	cout << "Possible Rooms: ";
	for (unordered_set<string>::iterator it = conf.Rooms.begin();it != conf.Rooms.end();++it) {
		cout << *it << " ";
	}
	cout << endl << "Possible Weapons: ";
	for (unordered_set<string>::iterator it = conf.Weapons.begin();it != conf.Weapons.end();++it) {
		cout << *it << " ";
	}
	cout << endl << "Possible Suspects: ";
	for (unordered_set<string>::iterator it = conf.Suspects.begin();it != conf.Suspects.end();++it) {
		cout << *it << " ";
	}
	cout << endl;

	cout << "-------------PRINT HANDS CALL FINISHED!!! -------------" << endl;
}