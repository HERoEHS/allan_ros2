#pragma once

#include <cctype>
#include <cstdlib>
#include <string>

namespace allan_ros {

// 파라미터로 받은 경로의 `~` 와 환경변수를 펼친다.
// rosbag2 는 받은 문자열을 그대로 열기 때문에, 셸이 해 주던 펼침을 여기서 대신한다.
// 그래야 같은 config 가 PC·로봇(사용자 이름이 다름)·install-only 배포에서 그대로 쓰인다.
//   "~" / "~/..."      → $HOME, $HOME/...  ("~user" 꼴은 펼치지 않는다)
//   "${VAR}" / "$VAR"  → 환경변수 값(없으면 빈 문자열)
// 명령 치환·단어 분리는 하지 않는다(wordexp 를 쓰지 않는 이유 — 공백 경로가 쪼개지지 않게).
inline std::string expand_path(const std::string & in)
{
  auto env = [](const std::string & name) -> std::string {
      const char * v = std::getenv(name.c_str());
      return v ? std::string(v) : std::string();
    };

  std::string s = in;
  if (!s.empty() && s[0] == '~' && (s.size() == 1 || s[1] == '/')) {
    s = env("HOME") + s.substr(1);
  }

  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] != '$' || i + 1 >= s.size()) {
      out += s[i];
      continue;
    }
    if (s[i + 1] == '{') {
      const size_t close = s.find('}', i + 2);
      if (close == std::string::npos) {  // 닫는 괄호가 없으면 글자 그대로 둔다
        out += s.substr(i);
        break;
      }
      out += env(s.substr(i + 2, close - i - 2));
      i = close;
      continue;
    }
    size_t j = i + 1;
    while (j < s.size() && (std::isalnum(static_cast<unsigned char>(s[j])) || s[j] == '_')) {
      ++j;
    }
    if (j == i + 1) {  // '$' 뒤에 이름이 없으면 글자 그대로
      out += s[i];
      continue;
    }
    out += env(s.substr(i + 1, j - i - 1));
    i = j - 1;
  }
  return out;
}

}  // namespace allan_ros
