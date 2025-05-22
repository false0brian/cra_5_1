#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "gmock/gmock.h"


using namespace std;

struct  Keyword {
	string name;
	int point;

	bool operator<(const Keyword& other) const {
		return point < other.point;
	}
};

vector<Keyword> topPerDay[7]; //월 ~ 일
vector<Keyword> topPerGroup[2]; //평일, 주말
int UZ = 9;

int getDayIndex(const string& wk) {
	static map<string, int> weekMap = {
		{"monday", 0}, {"tuesday", 1}, {"wednesday", 2},
		{"thursday", 3}, {"friday", 4}, {"saturday", 5}, {"sunday", 6}
	};
	return weekMap[wk];
}

int getGroupIndex(int weekIdx) {
	return (weekIdx <= 4) ? 0 : 1;
}

// 레벤슈타인 거리 계산 알고리즘 (문자열 유사도 검사)
int calcLevenshtein(const std::string& a, const std::string& b) {
	const size_t len_a = a.size();
	const size_t len_b = b.size();

	std::vector<std::vector<int>> d(len_a + 1, std::vector<int>(len_b + 1));

	for (size_t i = 0; i <= len_a; ++i) d[i][0] = i;
	for (size_t j = 0; j <= len_b; ++j) d[0][j] = j;

	for (size_t i = 1; i <= len_a; ++i) {
		for (size_t j = 1; j <= len_b; ++j) {
			if (a[i - 1] == b[j - 1])
				d[i][j] = d[i - 1][j - 1];
			else
				d[i][j] = 1 + std::min({ d[i - 1][j], d[i][j - 1], d[i - 1][j - 1] });
		}
	}
	return d[len_a][len_b];
}

// 점수 환산
bool isSimilar(const std::string& a, const std::string& b) {
	if (a.empty() && b.empty()) return true;
	if (a.empty() || b.empty()) return false;
	int dist = calcLevenshtein(a, b);
	// 유사도 비율 (1.0: 완전히 같음, 0.0: 전혀 다름)
	double similarity = 1.0 - static_cast<double>(dist) / std::max(a.length(), b.length());
	int score = 1 + static_cast<int>(similarity * 99);
	return score >= 80;
}

void resetScores() {
	UZ = 9;
	for (int i = 0; i < 7; ++i) {
		int num = 1;
		for (Keyword& node : topPerDay[i]) node.point = num++;
	}
	for (int i = 0; i < 2; ++i) {
		int num = 1;
		for (Keyword& node : topPerGroup[i]) node.point = num++;
	}
}

void updateOrInsert(vector<Keyword>& vec, const string& word, int point) {
	if (vec.size() < 10) {
		vec.push_back({ word, point });
	}
	else if (vec.back().point < point) {
		vec.pop_back();
		vec.push_back({ word, point });
	}
	std::sort(vec.begin(), vec.end());
	std::reverse(vec.begin(), vec.end());
}
bool checkResetScores(int pointUZ, int pointDay, int pointGroup) {
	if (pointUZ >= 2100000000)
		return true;
	else if (pointDay >= 2100000000)
		return true;
	else if (pointGroup >= 2100000000)
		return true;
	else
		return false;

}
string processInput(const string& w, const string& wk) {
	UZ++;
	int weekIdx = getDayIndex(wk);
	int weekendIdx = getGroupIndex(weekIdx);
	int point = UZ;
	int pointDay = 0;
	int pointGroup = 0;

	bool found = false;
	for (Keyword& node : topPerDay[weekIdx]) {
		if (node.name == w) {
			node.point += static_cast<int>(node.point * 0.1);
			pointDay = node.point;
			found = true;
			break;
		}
	}
	for (Keyword& node : topPerGroup[weekendIdx]) {
		if (node.name == w) {
			node.point += static_cast<int>(node.point * 0.1);
			pointGroup = node.point;
			found = true;
			break;
		}
	}

	if (checkResetScores(UZ, pointDay, pointGroup)) resetScores();
	if (found) return w;

	for (const Keyword& node : topPerDay[weekIdx]) {
		if (isSimilar(node.name, w)) return node.name;
	}
	for (const Keyword& node : topPerGroup[weekendIdx]) {
		if (isSimilar(node.name, w)) return node.name;
	}

	updateOrInsert(topPerDay[weekIdx], w, point);
	updateOrInsert(topPerGroup[weekendIdx], w, point);

	return w;
}

void loadInputData(const string& filename) {
	ifstream fin(filename);
	if (!fin.is_open()) {
		cerr << "파일 열기 실패: " << filename << endl;
		return;
	}
	string word, weekday;
	for (int i = 0; i < 500 && fin >> word >> weekday; ++i) {
		cout << processInput(word, weekday) << "\n";
	}
}

int main() {
	testing::InitGoogleMock();
	loadInputData("keyword_weekday_500.txt");
	return RUN_ALL_TESTS();
}
