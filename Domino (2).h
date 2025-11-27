/*  
    Authors: Yvonne Onmakpo, Alexander Miller
    Course:  CSCI 300 - Group 4
    Project: Dominoes Final Combo – Option 1 (Multithreaded Ready)
*/

#ifndef DOMINO_H
#define DOMINO_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>

using std::cout;
using std::string;
using std::vector;
using std::pair;

/*
 * CRandom
 * -------
 * Utility class for random operations: shuffling containers and
 * generating random integers in a closed interval [lo, hi].
 */
class CRandom {
public:
    // Shuffle a vector of domino pieces uniformly
    static void shuffle(vector<pair<int,int>>& v);

    // Return a random integer in [lo, hi] (inclusive)
    static int randint(int lo, int hi);
};

/*
 * CDominoes
 * ---------
 * Builds and owns the full set of 28 unique domino pieces (0–0 to 6–6).
 * On construction, the set is created and shuffled.
 */
class CDominoes {
    vector<pair<int,int>> all; // 28 pieces
public:
    CDominoes();                           // builds + shuffles full set
    const vector<pair<int,int>>& pieces() const; // read-only access
};

/*
 * CTable
 * ------
 * Represents the line ("train") of dominoes on the table.
 * Keeps track of the placed tiles in order so we can know
 * the current left and right ends for matching purposes.
 */
class CTable {
    vector<pair<int,int>> train; // placed tiles in order

public:
    bool empty() const { return train.empty(); }

    // Returns {leftEnd, rightEnd}; {-1, -1} if table is empty
    pair<int,int> ends() const;

    // Place a domino on left (toLeft = true) or right (toLeft = false)
    void place(pair<int,int> p, bool toLeft);

    // Print current state of the table to stdout
    void show() const;

    // Print a final game summary: winner, loser’s hand, final table
    void summary(const string& winner,
                 const vector<pair<int,int>>& loserHand) const;
};

/*
 * CPlayer
 * -------
 * Maintains the player's name and their hand of pieces.
 * Implements the logic for taking a turn: playing, drawing, passing.
 */
class CPlayer {
    string name;
    vector<pair<int,int>> hand;

    // Returns index of exact piece p in hand, or -1 if not found
    int indexOf(const pair<int,int>& p) const;

public:
    explicit CPlayer(string n) : name(std::move(n)) {}

    const string& getName() const { return name; }
    const vector<pair<int,int>>& getHand() const { return hand; }
    bool emptyHand() const { return hand.empty(); }

    // Give this player 10 pieces from the stock (taken from the back)
    void receive10(vector<pair<int,int>>& stock);

    /*
     * Decide if a move exists on the current table.
     * - If table is empty, any piece can start.
     * - Otherwise, chosen will hold an oriented piece that matches
     *   the left or right end, and toLeft tells which side to place on.
     * Returns true if a playable piece was found, false otherwise.
     */
    bool canPlay(const CTable& t, pair<int,int>& chosen, bool& toLeft) const;

    /*
     * Remove chosen from the player's hand (either in this orientation
     * or reversed), print the move, and place it on the table.
     */
    void playPiece(CTable& t, const pair<int,int>& chosen, bool toLeft);

    /*
     * Drawing phase: repeatedly draw from stock until a playable piece
     * is found or the stock is empty.
     * Returns true if a piece was eventually played, false otherwise.
     */
    bool drawThenPlay(vector<pair<int,int>>& stock, CTable& t);

    /*
     * One complete turn for this player:
     *  - If they can play immediately, do so.
     *  - Else attempt to draw and then play.
     *  - If still cannot play, they pass.
     *
     * Returns true if a piece was placed on the table, false if the
     * player ultimately passed.
     */
    bool takeTurn(CTable& t, vector<pair<int,int>>& stock);
};

#endif // DOMINO_H
