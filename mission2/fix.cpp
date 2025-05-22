#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include "gmock/gmock.h"


using namespace std;

class ISimilarityStrategy{
public:
	virtual ~ISimilarityStrategy() = default;
	virtual bool isSimilar(const string& a, const string& b) const = 0;
};

class LevenshteinStrategy : public ISimilarityStrategy {
public:

	bool isSimilar(const std::string& a, const std::string& b) const override{
		if (a.empty() && b.empty()) return true;
		if (a.empty() || b.empty()) return false;
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
		int dist = d[len_a][len_b];
		// 유사도 비율 (1.0: 완전히 같음, 0.0: 전혀 다름)
		double similarity = 1.0 - static_cast<double>(dist) / std::max(a.length(), b.length());
		int score = 1 + static_cast<int>(similarity * 99);
		return score >= 80;
	}
};
struct  Keyword {
	string name;
	int point;
	bool operator<(const Keyword& other) const {return point < other.point;}
};

class KeywordManager {

public:
	unique_ptr<ISimilarityStrategy> similarityStrategy;
	vector<Keyword> topPerDay[7];      // 인덱스 0~6: monday~sunday
	vector<Keyword> topPerGroup[2];    // 인덱스 0: 평일, 1: 주말
	int point, pointDay, pointGroup;

	KeywordManager() : similarityStrategy(make_unique<LevenshteinStrategy>()), point(9), pointDay(0), pointGroup(0) {}

	void setSimilarityStrategy(unique_ptr<ISimilarityStrategy> newOne) {
		similarityStrategy = move(newOne);
	}
	
	void setPoint(int target) { point = target; }
	int getPoint() { return point; }
	void loadKeywords(const string& filename, int maxRecords = 500) {
		ifstream fin(filename);
		/*if (!fin) {
			cerr << "파일 열기 실패: " << filename << '\n';
			return ;
		}*/
		string word, weekday;
		
		for (int i = 0; i < maxRecords && (fin >> word >> weekday); ++i) {
			cout << processInput(word, weekday) << '\n';
		}
	}

	void resetVar() {
		point = 9;
		pointDay = 0;
		pointGroup = 0;
	}

	bool updateExisting(vector<Keyword>& vec, const string& w, int& outPoint) {
		for (auto& kw : vec) {
			if (kw.name == w) {
				kw.point += int(kw.point * 0.1);
				outPoint = kw.point;
				return true;
			}
		}
		return false;
	}

	string processInput(const string& w, const string& weekday) {
		++point;
		
		int dayIdx = getDayIndex(weekday);
		int grpIdx = getGroupIndex(dayIdx);
		int score = point;

		// 기존 키워드 업데이트
		if (updateExisting(topPerDay[dayIdx], w, pointDay) || updateExisting(topPerGroup[grpIdx], w, pointGroup)) {
			if (checkResetScores(point, pointDay, pointGroup)) resetScores();
			return w;
		}

		// 유사도 키워드 검색
		for (auto& kw : topPerDay[dayIdx]) {
			if (similarityStrategy->isSimilar(kw.name, w)) return kw.name;
		}
		for (auto& kw : topPerGroup[grpIdx]) {
			if (similarityStrategy->isSimilar(kw.name, w)) return kw.name;
		}

		// 신규 키워드 추가/교체
		updateOrInsert(topPerDay[dayIdx], w, score);
		updateOrInsert(topPerGroup[grpIdx], w, score);
		if (checkResetScores(point, pointDay, pointGroup)) resetScores();
		return w;
	}

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

	void resetScores() {
		resetVar();
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
		else if (pointDay >= 2100000000 || pointDay < 0)
			return true;
		else if (pointGroup >= 2100000000 || pointGroup < 0)
			return true;
		else
			return false;

	}
};
