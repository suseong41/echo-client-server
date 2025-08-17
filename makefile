CXX      = g++
CXXFLAGS = -std=c++23 -Wall -O2
LDFLAGS  = -pthread

OUT      = output

CLIENT = $(OUT)/suseong-echo-client
SERVER = $(OUT)/suseong-echo-server

all: $(CLIENT) $(SERVER)

$(OUT):
	mkdir -p $@

$(CLIENT): suseong-echo-client/client-start.cpp | $(OUT)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(SERVER): suseong-echo-server/server-start.cpp | $(OUT)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(OUT)