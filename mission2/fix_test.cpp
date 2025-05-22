#include "gmock/gmock.h"
#include "fix.cpp"


using namespace testing;
using namespace std;
// 레벤슈타인 거리 검증

class MockKeywordManager : public KeywordManager {
};

 //유사도 함수 검증
TEST(KeywordManagerTest, TC_isSimilar) {
    MockKeywordManager mkm;
    EXPECT_EQ(mkm.similarityStrategy->isSimilar("test", "test"), true);
    EXPECT_EQ(mkm.similarityStrategy->isSimilar("test", "tent"), false);
    EXPECT_EQ(mkm.similarityStrategy->isSimilar("apple", "appl"), true);
    EXPECT_EQ(mkm.similarityStrategy->isSimilar("", "anything"), false);
}

 //요일→인덱스 변환 검증
TEST(KeywordManagerTest, TC_getDayIndex) {
    MockKeywordManager mkm;
    int ret = mkm.getDayIndex("monday");
    EXPECT_EQ(ret, 0);
    ret = mkm.getDayIndex("wednesday");
    EXPECT_EQ(ret, 2);
    ret = mkm.getDayIndex("sunday");
    EXPECT_EQ(ret, 6);
    ret = mkm.getDayIndex("unknown");
    EXPECT_EQ(ret, 0);  // default
}

// 평일/주말 그룹 인덱스 검증
TEST(KeywordManagerTest, TC_getGroupIndex) {
    MockKeywordManager mkm;
    EXPECT_EQ(mkm.getGroupIndex(0), 0);  // monday → 평일
    EXPECT_EQ(mkm.getGroupIndex(4), 0);  // friday → 평일
    EXPECT_EQ(mkm.getGroupIndex(5), 1);  // saturday → 주말
    EXPECT_EQ(mkm.getGroupIndex(6), 1);  // sunday → 주말
}

// processKeyword 기본 흐름 검증
TEST(KeywordManagerTest, TC_processInput) {
    // 새로운 키워드 등록
    MockKeywordManager mkm;
    string out1 = mkm.processInput("apple", "monday");
    EXPECT_EQ(out1, "apple");
    // 두 번 째 호출은 updateExisting되어 동일 반환
    string out2 = mkm.processInput("apple", "monday");
    EXPECT_EQ(out2, "apple");
    // 약간 변형된 단어는 유사도 매칭 → "apple" 반환
    string out3 = mkm.processInput("appl", "monday");
    EXPECT_EQ(out3, "apple");
}

// 스코어 한계 도달 시 리셋 동작 검증
TEST(KeywordManagerTest, TC_resetScores_topPerDay) {
    // 전역 카운터를 최대치 근처로 설정
    MockKeywordManager mkm;
    mkm.setPoint(2100000001);
    // 한 번만 호출해도 resetScores가 작동하여 counter가 초기화됨
    string out = mkm.processInput("banana", "tuesday");
    EXPECT_EQ(mkm.getPoint(), 9);  
    EXPECT_EQ(out, "banana");
    mkm.processInput("apple", "tuesday");
    EXPECT_EQ(mkm.getPoint(), 10);
    mkm.setPoint(2099999009);
    out = mkm.processInput("banana", "tuesday");
    EXPECT_EQ(out, "banana");
    mkm.processInput("carrot", "tuesday");
    mkm.processInput("dog", "tuesday");
    mkm.processInput("edge", "tuesday");
    mkm.processInput("father", "tuesday");
    mkm.processInput("google", "tuesday");
    mkm.processInput("edge", "saturday");
    mkm.processInput("father", "saturday");
    mkm.processInput("google", "saturday");
    mkm.processInput("hit", "tuesday");
    out = mkm.processInput("carrot", "tuesday");
    EXPECT_EQ(mkm.getPoint(), 9);
    EXPECT_EQ(out, "carrot");
    mkm.processInput("internet", "tuesday");
    mkm.processInput("dodo", "tuesday");
    mkm.processInput("bobo", "tuesday");
    out = mkm.processInput("zebra", "tuesday");
    EXPECT_EQ(out, "zebra");
}

TEST(KeywordManagerTest, TC_resetScores_topPerGroup) {
    // 전역 카운터를 최대치 근처로 설정
    MockKeywordManager mkm;
    mkm.setPoint(2100000001);
    // 한 번만 호출해도 resetScores가 작동하여 counter가 초기화됨
    string out = mkm.processInput("banana", "saturday");
    EXPECT_EQ(mkm.getPoint(), 9);
    EXPECT_EQ(out, "banana");
    mkm.processInput("apple", "saturday");
    EXPECT_EQ(mkm.getPoint(), 10);
    mkm.setPoint(2099999009);
    out = mkm.processInput("banana", "sunday");
    EXPECT_EQ(out, "banana");
    mkm.processInput("carrot", "saturday");
    mkm.processInput("dog", "sunday");
    mkm.processInput("edge", "sunday");
    mkm.processInput("father", "sunday");
    mkm.processInput("google", "sunday");
    mkm.processInput("edge", "saturday");
    mkm.processInput("father", "saturday");
    mkm.processInput("google", "saturday");
    mkm.processInput("hit", "saturday");
    out = mkm.processInput("carrot", "sunday");
    EXPECT_EQ(mkm.getPoint(), 13);
    EXPECT_EQ(out, "carrot");
    mkm.processInput("internet", "saturday");
    mkm.processInput("dodo", "saturday");
    mkm.processInput("bobo", "saturday");
    out = mkm.processInput("zebra", "saturday");
    EXPECT_EQ(out, "zebra");
}

//TEST(KeywordManagerTest, TC_loadKeywords) {
//    MockKeywordManager mkm;
//    string ret = mkm.loadKeywords("keyword_weekday_500.txt", 1);
//    EXPECT_EQ(ret, "water");
//}

int main() {
    testing::InitGoogleMock();
    MockKeywordManager manager;
    auto start = make_unique<LevenshteinStrategy>();
    manager.loadKeywords("keyword_weekday_500.txt");
    return RUN_ALL_TESTS();
}
