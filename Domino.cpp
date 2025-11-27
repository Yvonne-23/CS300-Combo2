#include "Domino.h"

// ---------------- CRandom ----------------

void CRandom::shuffle(vector<pair<int,int>>& v) {
    // Local generator avoids shared-state issues if threads are used elsewhere
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(v.begin(), v.end(), gen);
}

int CRandom::randint(int lo, int hi) {
    // Simple local engine + distribution; uniform in [lo, hi]
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(lo, hi);
    return dist(gen);
}

// ---------------- CDominoes ----------------

CDominoes::CDominoes() {
    all.clear();
    // Create all unique tiles (i, j) with 0 <= i <= j <= 6
    for (int i = 0; i <= 6; ++i) {
        for (int j = i; j <= 6; ++j) {
            all.push_back({i, j});
        }
    }
    CRandom::shuffle(all);
}

const vector<pair<int,int>>& CDominoes::pieces() const {
    return all;
}

// ---------------- CTable ----------------

pair<int,int> CTable::ends() const {
    if (train.empty()) return {-1, -1};
    // left end = first tile's left pip, right end = last tile's right pip
    return { train.front().first, train.back().second };
}

void CTable::place(pair<int,int> p, bool toLeft) {
    if (train.empty()) {
        // First piece simply starts the train
        train.push_back(p);
        return;
    }

    if (toLeft) {
        train.insert(train.begin(), p);
    } else {
        train.push_back(p);
    }
}

void CTable::show() const {
    cout << "Table: ";
    for (const auto& p : train) {
        cout << "[" << p.first << "|" << p.second << "] ";
    }
    cout << "\n";
}

void CTable::summary(const string& winner,
                     const vector<pair<int,int>>& loserHand) const {
    cout << "\n===== GAME SUMMARY =====\n";
    cout << "Winner: " << winner << "\n";

    cout << "Loser remaining (" << loserHand.size() << "): ";
    for (const auto& p : loserHand) {
        cout << "[" << p.first << "|" << p.second << "] ";
    }
    cout << "\n";

    cout << "Final table: ";
    for (const auto& p : train) {
        cout << "[" << p.first << "|" << p.second << "] ";
    }
    cout << "\n========================\n";
}

// ---------------- CPlayer ----------------

int CPlayer::indexOf(const pair<int,int>& p) const {
    for (int i = 0; i < static_cast<int>(hand.size()); ++i) {
        if (hand[i] == p) return i;
    }
    return -1;
}

void CPlayer::receive10(vector<pair<int,int>>& stock) {
    // Assumes stock has at least 10 pieces
    for (int i = 0; i < 10; ++i) {
        hand.push_back(stock.back());
        stock.pop_back();
    }
}

bool CPlayer::canPlay(const CTable& t, pair<int,int>& chosen, bool& toLeft) const {
    if (t.empty()) {
        // Any tile can start if table is empty
        if (hand.empty()) return false;
        chosen = hand.front();
        toLeft = false;
        return true;
    }

    auto [L, R] = t.ends();

    for (auto p : hand) {
        // Try matching right end
        if (p.first == R) {
            chosen = p;
            toLeft = false;
            return true;
        }
        if (p.second == R) {
            chosen = { p.second, p.first }; // flip piece
            toLeft = false;
            return true;
        }

        // Try matching left end
        if (p.second == L) {
            chosen = p;
            toLeft = true;
            return true;
        }
        if (p.first == L) {
            chosen = { p.second, p.first }; // flip piece
            toLeft = true;
            return true;
        }
    }
    return false; // no playable piece found
}

void CPlayer::playPiece(CTable& t, const pair<int,int>& chosen, bool toLeft) {
    int idx = indexOf(chosen);
    if (idx == -1) {
        // Maybe we stored it reversed in hand
        pair<int,int> rev{ chosen.second, chosen.first };
        idx = indexOf(rev);
    }

    if (idx != -1) {
        hand.erase(hand.begin() + idx);
    }

    cout << name << " plays [" << chosen.first << "|" << chosen.second << "] "
         << (toLeft ? "to LEFT" : "to RIGHT") << "\n";

    t.place(chosen, toLeft);
    t.show();
}

bool CPlayer::drawThenPlay(vector<pair<int,int>>& stock, CTable& t) {
    while (!stock.empty()) {
        auto d = stock.back();
        stock.pop_back();
        hand.push_back(d);

        cout << name << " draws [" << d.first << "|" << d.second << "]\n";

        pair<int,int> ch;
        bool toLeft = false;
        if (canPlay(t, ch, toLeft)) {
            playPiece(t, ch, toLeft);
            return true; // successfully played after draw
        }
    }
    return false; // stock empty or still no playable tile
}

bool CPlayer::takeTurn(CTable& t, vector<pair<int,int>>& stock) {
    cout << "\n-- " << name << "'s turn --\n";

    pair<int,int> ch;
    bool toLeft = false;

    // Try to play immediately
    if (canPlay(t, ch, toLeft)) {
        playPiece(t, ch, toLeft);
        return true;
    }

    // Otherwise try drawing then playing
    bool playedAfterDraw = drawThenPlay(stock, t);
    if (!playedAfterDraw) {
        cout << name << " passes.\n";
    }
    return playedAfterDraw;
}
