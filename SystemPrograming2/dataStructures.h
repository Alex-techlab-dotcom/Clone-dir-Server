#include <list>
#include <utility>
#include <string>
using namespace std;

class Queue {
private:
    list<pair<string,int>> availableWorkers;
    int maxSize;
    int currentSize=0;
public:
    void Insert(const pair<string,int>& filenameWithFD) {
        this->availableWorkers.push_front(filenameWithFD);
        currentSize++;
    }

    pair<string,int> PopOut() {
        pair<string,int> filenameWithFD = this->availableWorkers.back();
        this->availableWorkers.pop_back();
        currentSize--;
        return filenameWithFD;
    }

    bool IsEmpty() {
        return this->availableWorkers.empty();
    }
    void SetMaxSize(int _maxSize){
        maxSize=_maxSize;
    }
    int GetSize() const{
        return currentSize;
    }
    bool isFull() const{
        return (maxSize==currentSize);
    }
};
