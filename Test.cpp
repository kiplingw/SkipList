// Includes...

    // System...
    #include <cassert>
    #include <iostream>
    #include <map>
    #include <random>
    #include <string>
    #include <utility>
    #include <vector>

    // Our headers...
    #include "SkipList.h"

// Use the standard namespace...
using namespace std;

// Entry point...
int main()
{
    // Create a skip list that maps integers to strings...
    SkipList<int, string> List;
//    map<int, string> List;

    // Initialize random generator...

        // Random device to feed the generator...
        std::random_device      RandomDevice;

        // Mersenne Twister random generator from the random device...
        std::mt19937            RandomGenerator;

    // Maximum integer to insert...
    const int MaximumInteger = 100000;

    // Generate a random pool of unique integers...

        // Allocate enough storage for all integers...
        vector<int> RandomIntegers(MaximumInteger);

        // Fill with sequentially ordered integers...
        iota(begin(RandomIntegers), end(RandomIntegers), 1);

        // Randomly shuffle the integers...
        shuffle(begin(RandomIntegers), end(RandomIntegers), RandomGenerator);

    // Insert each integer key into the list with its string representation as
    //  the value...
    for_each(cbegin(RandomIntegers), cend(RandomIntegers), [&List](const int &Key)
    {
        if(Key % 10000 == 0)
            cout << "." << flush;
//        cout << Key << endl;

        //List.insert(make_pair(Key, to_string(Key)));
        List.Insert(Key, to_string(Key));
    });

    // Check size...
    assert(List.GetSize() == MaximumInteger);

    cout << "Searching for 5..." << endl;
    const auto Iterator = List.Search(5);
    if(Iterator != cend(List))
        cout << "Found: " << Iterator->second << endl;
    else
        cout << "Not found!" << endl;

    cout << "Clearing..." << endl;
    List.Clear();

    // Check size...
    assert(List.GetSize() == 0);

    return 0;
}

