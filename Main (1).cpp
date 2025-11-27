// Author: Alexander Miller
// Final Combo Project – Multithreaded Dominoes

#include "Domino.h"
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv_p1, cv_p2;

bool p1Turn;
bool gameOver = false;
bool ready = false;

void playerThread(CPlayer& self, CPlayer& other,
                  CTable& table, vector<pair<int,int>>& bank,
                  std::condition_variable& myCV,
                  std::condition_variable& otherCV,
                  bool isP1)
{
    std::unique_lock<std::mutex> lock(mtx);

    while (!gameOver)
    {
        myCV.wait(lock, [&]{ return ready && (p1Turn==isP1 || gameOver); });
        if (gameOver) break;

        bool played = self.takeTurn(table, bank);
        if (self.emptyHand())
        {
            table.summary(self.getName(), other.getHand());
            gameOver = true;
            otherCV.notify_all();
            break;
        }

        // If both players pass twice in a row, game ends
        static int passes = 0;
        passes = played ? 0 : passes + 1;
        if (passes >= 2)
        {
            cout << "\nBoth players passed twice. Game ends.\n";
            table.show();
            gameOver = true;
            otherCV.notify_all();
            break;
        }

        // Switch turn
        p1Turn = !p1Turn;
        ready = true;
        otherCV.notify_all();
    }
}

int main() {
    CDominoes set;
    vector<pair<int,int>> bag = set.pieces();

    CPlayer p1("Player 1"), p2("Player 2");
    CTable table;

    p1.receive10(bag);
    p2.receive10(bag);
    vector<pair<int,int>> stock = bag;

    p1Turn = (CRandom::randint(0,1)==0);
    cout << (p1Turn ? "Player 1" : "Player 2") << " starts (THREAD MODE)\n";

    ready = true;

    std::thread t1(playerThread, std::ref(p1), std::ref(p2),
                   std::ref(table), std::ref(stock),
                   std::ref(cv_p1), std::ref(cv_p2), true);

    std::thread t2(playerThread, std::ref(p2), std::ref(p1),
                   std::ref(table), std::ref(stock),
                   std::ref(cv_p2), std::ref(cv_p1), false);

    {
        std::lock_guard<std::mutex> lg(mtx);
        cv_p1.notify_all();
        cv_p2.notify_all();
    }

    t1.join();
    t2.join();

    cout << "\n=== GAME COMPLETE ===\n";
    return 0;
}
