#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

#include "allan_ros2/path_utils.hpp"

using allan_ros::expand_path;

class ExpandPath : public ::testing::Test
{
protected:
  void SetUp() override
  {
    setenv("HOME", "/home/tester", 1);
    setenv("EDIE_CALIB_DIR", "/data/calib", 1);
    unsetenv("EDIE_NOT_SET");
  }
};

// 홈 폴더 펼침 — 같은 config 가 사용자 이름이 다른 장비에서 그대로 쓰이는 이유
TEST_F(ExpandPath, TildeSlashBecomesHome)
{
  EXPECT_EQ(expand_path("~/.edie/calib/imu_static_2h"), "/home/tester/.edie/calib/imu_static_2h");
  EXPECT_EQ(expand_path("~"), "/home/tester");
}

// "~user" 꼴과 중간의 '~' 는 건드리지 않는다
TEST_F(ExpandPath, TildeElsewhereIsLiteral)
{
  EXPECT_EQ(expand_path("~other/bag"), "~other/bag");
  EXPECT_EQ(expand_path("/a/~/b"), "/a/~/b");
}

TEST_F(ExpandPath, EnvVarBracedAndBare)
{
  EXPECT_EQ(expand_path("${EDIE_CALIB_DIR}/imu_static_2h"), "/data/calib/imu_static_2h");
  EXPECT_EQ(expand_path("$EDIE_CALIB_DIR/imu_static_2h"), "/data/calib/imu_static_2h");
  EXPECT_EQ(expand_path("$HOME/x"), "/home/tester/x");
}

// 없는 변수는 빈 문자열(셸과 같음)
TEST_F(ExpandPath, UnsetVarIsEmpty)
{
  EXPECT_EQ(expand_path("/a/${EDIE_NOT_SET}/b"), "/a//b");
}

// 펼칠 것이 없으면 그대로 — 기존 절대경로 config 는 동작이 바뀌지 않는다
TEST_F(ExpandPath, PlainPathUnchanged)
{
  EXPECT_EQ(expand_path("/home/higony/bag.db3"), "/home/higony/bag.db3");
  EXPECT_EQ(expand_path("bag"), "bag");
  EXPECT_EQ(expand_path(""), "");
  EXPECT_EQ(expand_path("/path with space/bag"), "/path with space/bag");
}

// 잘못된 꼴은 글자 그대로 남긴다(조용히 다른 경로를 만들지 않게)
TEST_F(ExpandPath, MalformedLeftLiteral)
{
  EXPECT_EQ(expand_path("/a/${UNCLOSED"), "/a/${UNCLOSED");
  EXPECT_EQ(expand_path("/a/$/b"), "/a/$/b");
  EXPECT_EQ(expand_path("cost$"), "cost$");
}
