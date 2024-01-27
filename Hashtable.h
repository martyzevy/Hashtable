#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <queue>

template<typename T>
class Hashtable
{
	public:

		Hashtable(bool debug = false, unsigned int size = 11);

		~Hashtable();

		int add(std::string k, const T& val);

		const T& lookup(std::string k) const;

		void reportAll(std::ostream& out) const;

		int hash(std::string k) const;

	private:
		void resize();
		std::pair<std::string, T>* arr;	//Array storing key, value pairs
		double load;
		bool* tracker;	//Stores which indices already contain key, value pairs
		void generateRandom();
		int m;
		long long r[5];	//Contains randomly generated numbers
		bool checkPrime(int val);
		std::queue<int> sizes;
};

template<typename T>
Hashtable<T>::Hashtable(bool debug, unsigned int size){
	arr = new std::pair<std::string, T>[size];	//Array that will store key, value pairs
	m = size;
	load = 0;

	/*Tracker array stores which indices contain key, value pairs. Initialize to false
	because array is initially empty*/
	tracker = new bool[size];
	for (int i = 0; i < m; i++){
		tracker[i] = false;
	} 

	//Add all of the future array sizes to the queue, these will be used in the resize method
	int arr_sizes[17] = {11, 23, 47, 97, 197, 397, 797, 1597, 3203, 6421, 12853, 25717, 51437, 102877, 205759, 411527, 823117};
	int i = 0;
	while (i < 17){
		if (arr_sizes[i] > m) sizes.push(arr_sizes[i]);
		i++;
	}

	if (debug == true){
		r[0] = 983132572;
		r[1] = 1468777056;
		r[2] = 552714139;
		r[3] = 984953261;
		r[4] = 261934300;
	}
	else generateRandom();
}

template<typename T>
Hashtable<T>::~Hashtable(){
	delete [] arr;
	delete [] tracker;
}

template<typename T>
int Hashtable<T>::hash(std::string k) const{
	long long nums[5];
	int idx = k.size() - 1 - 5;	//Start at the sixth-to-last character
	int arr_i = 4;

	while (idx >= 0 && arr_i >= 0){
		long long val = 0;
		for (int j = 0; j < 6; j++){
			//Convert each letter to an integer and add that integer to val
			val += pow(27, (5 - j)) * (static_cast<int>(k[idx]) - 96);
			idx++;
		}
		idx -= 12;	//Move to the beggining of the next 6-letter substring
		nums[arr_i] = val;
		arr_i--;	
	}

	//If there were less than 6 remaining characters, store values for the remaining characters
	if (idx < 0){
		long long val = 0;
		int exponent = 5 - abs(idx);
		for (int i = 0; i < 5 - abs(idx) + 1; i++){
			val += pow(27, exponent) * (static_cast<int>(k[i]) - 96);
			exponent--;
		}
		nums[arr_i] = val;
	}

	//Place zeros in the leading positions of the array if the word was shorter than 25 letters
	for (int i = 0; i < arr_i; i++){
		nums[i] = 0;
	} 

	//Hash the word by multiplying each value in the array by one of the Hashtable's randomly generated values
	long long hashVal = 0;
	for (int i = 0; i < 5; i++){
		hashVal += r[i] * nums[i];
	}

	return hashVal % m;
}

template<typename T>
void Hashtable<T>::generateRandom(){
	for (int i = 0; i < 5; i++){
		r[i] = rand() % m;	//Get 5 random values between 0 and m-1
	}
}

template<typename T>
int Hashtable<T>::add(std::string k, const T& val){
	//Check if the key already exists
	int index = hash(k);
	int probe = 0;
	while (tracker[index] == true){
		if (arr[index].first == k){
			//Update the value
			arr[index].second = val;	
			return 0;
		}
		probe++;
		index = (hash(k) + static_cast<int>(pow(probe, 2))) % m;
	}

	//Check if adding this element would trigger a resize
	bool resized = false;
	if ((load + 1) / m > 0.5){
		resized = true;
		resize();
	} 

	//If the table was resized, find the new index for the key (use quadratic probing for collisions)
	if (resized == true){
		index = hash(k);
		probe = 0;
		while (tracker[index] == true){
			probe++;
			index = (hash(k) + static_cast<int>(pow(probe, 2))) % m;
		}
	}
	
	//Add key, value pair to the array
	arr[index] = std::pair<std::string, T>(k, val);
	tracker[index] = true;
	load++;

	return probe;
}

template<typename T>
void Hashtable<T>::resize(){
	int oldSize = m;
	int newSize;

	//Get the next size in the sequence provided
	if (!sizes.empty()){
		newSize = sizes.front();
		sizes.pop();
	}
	//If all sizes in the provided sequence have been used, double the size and get the next prime
	else{
		newSize = oldSize * 2;
		while (checkPrime(newSize) == false){
			newSize++;
		}
	}
	
	//Copy everything over
	std::vector<std::pair<std::string, T>> temp;
	for (int i = 0; i < oldSize; i++){
		if (tracker[i] == true){
			temp.push_back(arr[i]);
		}
	} 

	//Delete the old data and allocate a new array of the new size
	delete [] arr;
	delete [] tracker;
	arr = new std::pair<std::string, T>[newSize];
	m = newSize;

	//Populate the new array with the new hash values
	load = 0;
	tracker = new bool[m];
	for (int i = 0; i < m; i++){
		tracker[i] = false;
	}
	for (unsigned int i = 0; i < temp.size(); i++){
		add(temp[i].first, temp[i].second);
	}
}

template<typename T>
const T& Hashtable<T>::lookup(std::string k) const{
	int idx = hash(k);
	if (tracker[idx] == false) return arr[0].second;	//Check if key does not exist

	//Find the index associated with this key
	int collisions = 0;
	while (arr[idx].first != k){
		collisions++;
		idx = (hash(k) + static_cast<int>(pow(collisions, 2))) % m;
	}

	return arr[idx].second;
}

template<typename T>
void Hashtable<T>::reportAll(std::ostream& out) const{
	for (int i = 0; i < m; i++){
		//Only print indices that contain key, value pairs
		if (tracker[i] == true){
			out << arr[i].first << " " << arr[i].second << std::endl;
		}
	}
}

template<typename T>
bool Hashtable<T>::checkPrime(int val){
	if (val < 2 || val % 2 == 0) return false;	//Even numbers cannot be prime
	for (int i = 2; i < sqrt(val); i++){	//Only need to check up to sqrt(val)
		if (val % i == 0) return false;
	}
	return true;
}

#endif
