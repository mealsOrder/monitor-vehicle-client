# 컴파일러
CXX = g++

# 컴파일 옵션
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 `pkg-config --cflags opencv4`

# 라이브러리 링크
LDFLAGS = `pkg-config --libs opencv4` -lboost_system -lboost_thread

# 대상 실행 파일
TARGET = servertest

# 소스 파일
SRC = servertest.cpp

# 객체 파일
OBJ = $(SRC:.cpp=.o)

# 기본 빌드 대상
all: $(TARGET)

# 실행 파일 생성
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 개별 소스 파일 컴파일
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 클린 명령
clean:
	rm -f $(OBJ) $(TARGET)

