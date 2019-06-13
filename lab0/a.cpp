#include <iostream>
#include <list>
#include <string>
#include <iterator>
#include <algorithm>
#include <random>

using namespace std;

struct Node
{
    string key;
    string value;
    Node(string key, string value) : key(key), value(value)
    {
    }
};

class Multimap
{
public:
    list<Node> data;

    bool insert(string key, string value)
    {
        int prevSize = data.size();
        data.push_back(Node(key, value));
        bool result = false;
        if (data.size() == 1 + prevSize)
            result = true;
        // cout << "insert " << key << " " << value << " " << result << endl;
        return result;
    }

    bool find(string key)
    {
        bool result = false;
        for (const Node &node : data)
        {
            if (node.key == key)
                result = true;
        }
        // cout << "find " << key << " " << result << endl;
        return result;
    }

    bool find(string key, list<string> &values)
    {
        bool contains = find(key);
        bool result = false;
        if (contains)
        {
            for (const Node &node : data)
            {
                if (node.key == key)
                {
                    values.push_back(node.value);
                }
            }
        }
        return result;
    }

    int remove(string key)
    {
        int result = -1;
        bool contains = find(key);
        list<Node> newData;
        if (contains)
        {
            result = 0;
            for (const Node &node : data)
            {
                if (node.key != key)
                {
                    newData.push_back(node);
                    result++;
                    cout << "removing " << key << " " << node.value << endl;
                }
            }
            data.clear();
            data.insert(data.end(), newData.begin(), newData.end());
        }
        cout << "removed total " << result << endl;
        for (const Node &node : data)
        {
            cout << "new list " << key << " " << node.value << endl;
        }
        return result;
    }
};

int main()
{
    Multimap map;
    // list<string> values;
    // map.insert("new", "1");
    // map.find("new");
    // map.insert("new", "2");
    // map.find("new", values);
    // for (const string &string : values)
    //     cout << string << endl;
    // map.remove("new");

    cout << "tests:" << endl;

    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> dist(0, 200);

    for (int j = 0; j < 10; j++)
    {
        list<string> numbers;
        for (int i = 0; i < 100; i++)
        {
            string key = to_string(dist(rng));
            string value = to_string(dist(rng));
            map.insert(key, value);
            numbers.push_back(key);
        }
        string result = "passed";
        for (const string &num : numbers) {
            if (!map.find(num)) result = "failed";
        }
        cout << "test " << j+1 << " " << result << endl;
    }
}