#include "RecommendManager.h"
#include "../General/Math.h"
#include "../General/Timing.h"

#include "./Example.h"

#include <vector>
#include <iostream>

using namespace std;

#define INTERVAL 5000

double Point::C = 1.0;

RecommendManager* manager = new RecommendManager();

int x = 0;

JNIEXPORT void JNICALL Java_Example_initLSH(JNIEnv *env, jobject obj, jint numItems){
	manager->init(numItems);
	x += 1;
}

JNIEXPORT void JNICALL Java_Example_addLSHPoint(JNIEnv *env, jobject obj, jintArray items, jintArray ratings){

	x *= 3;
	List<Pair>* userRatings = new List<Pair>();
	jsize len = env->GetArrayLength(items);
    jint *itemsBody = env->GetIntArrayElements(items, 0);
	jint *ratingsBody = env->GetIntArrayElements(ratings, 0);

	for (int i = 0; i < len; i++) {
		userRatings->insertAtEnd(new Pair(itemsBody[i], ratingsBody[i]), true);
		// std::cout << "itemsBody[" << i << "] = " << itemsBody[i] << '\n';
		// std::cout << "ratingsBody[" << i << "] = " << ratingsBody[i] << '\n';
		// std::cout << '\n';
	}
	manager->addPoint(userRatings);

	env->ReleaseIntArrayElements(items, itemsBody, 0);
	env->ReleaseIntArrayElements(ratings, ratingsBody, 0);
}

JNIEXPORT jintArray JNICALL Java_Example_getRecommendedItems(JNIEnv *env, jobject obj, jint userIndex, jint top){
	// vector<int> recommendedCPP;
	// recommendedCPP.push_back(x);
	vector<int> recommendedCPP = manager->evaluate(userIndex, top);
	// manager->print();
	unsigned int length = recommendedCPP.size();
	jintArray recommendedJava = env->NewIntArray(length);
	env->SetIntArrayRegion(recommendedJava, 0, length, &(recommendedCPP[0]));
	return recommendedJava;
}

int main(int argc, char *argv[]) {
	// srand(time(NULL));
	// setup(INTERVAL);
	//
	// std::cout << "OK!" << '\n';
	//
	// int numItems = 100;
	// int numUsers = 7;
	//
	// manager->init(numItems);
	// for (int i = 0; i < numUsers; i++) {
	// 	List<Pair>* ratings = new List<Pair>();
	// 	for (int j = 0; j < i; j++) {
	// 		ratings->insertAtEnd(new Pair(j, i % 3), true);
	// 	}
	// 	manager->addPoint(ratings);
	// }
	//
	// int user = 0;
	// vector<int> top5 = manager->evaluate(user, 5);
	// std::cout << "products: " << '\n';
	// for (size_t i = 0; i < top5.size(); i++) {
	// 	std::cout << top5[i] << '\n';
	// }

	return 0;
}
