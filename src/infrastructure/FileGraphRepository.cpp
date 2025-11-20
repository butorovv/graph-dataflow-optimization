#include "infrastructure/FileGraphRepository.h"
#include "domain/NetworkGraph.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <windows.h>

namespace Infrastructure {

static std::string cp1251_to_utf8(const std::string& src) {
    if (src.empty()) return {};
    int wlen = MultiByteToWideChar(1251, 0, src.data(), static_cast<int>(src.size()), nullptr, 0);
    if (wlen == 0) return {};
    std::vector<wchar_t> wbuf(wlen);
    MultiByteToWideChar(1251, 0, src.data(), static_cast<int>(src.size()), wbuf.data(), wlen);
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, nullptr, 0, nullptr, nullptr);
    if (u8len == 0) return {};
    std::vector<char> u8buf(u8len);
    WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, u8buf.data(), u8len, nullptr, nullptr);
    return std::string(u8buf.data(), u8len);
}

Domain::NetworkGraphPtr FileGraphRepository::loadGraph(const std::string& filename) {
    auto graph = std::make_shared<Domain::NetworkGraph>("Facebook Social Network");

    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::string raw;
    in.seekg(0, std::ios::end);
    raw.resize(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::ios::beg);
    in.read(&raw[0], static_cast<std::streamsize>(raw.size()));
    in.close();

    std::string text;
    bool isUtf8 = false;
    if (raw.size() >= 3 &&
        static_cast<unsigned char>(raw[0]) == 0xEF &&
        static_cast<unsigned char>(raw[1]) == 0xBB &&
        static_cast<unsigned char>(raw[2]) == 0xBF) {
        isUtf8 = true;
        text = raw.substr(3);
    } else {
        size_t high = 0;
        for (unsigned char c : raw) if (c >= 0x80) ++high;
        if (high > raw.size() / 20) {
            text = cp1251_to_utf8(raw);
        } else {
            text = raw;
        }
    }

    // Split into lines (handle CRLF)
    std::vector<std::string> lines;
    {
        std::string cur;
        for (size_t i = 0; i < text.size(); ++i) {
            char c = text[i];
            if (c == '\r') continue;
            if (c == '\n') {
                lines.push_back(cur);
                cur.clear();
            } else cur.push_back(c);
        }
        if (!cur.empty()) lines.push_back(cur);
    }

    // Log first 10 lines for debug
    std::cout << "Показ первых строк файла (" << filename << "):\n";
    for (size_t i = 0; i < lines.size() && i < 10; ++i) {
        std::cout << (i+1) << ": " << lines[i] << "\n";
    }

    auto trim = [](std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    };

    int edges_loaded = 0;
    int unparsable = 0;
    int max_node_id = 0;
    int line_no = 0;

    for (const auto& rawline : lines) {
        ++line_no;
        std::string line = rawline;
        trim(line);
        if (line.empty()) continue;
        if (line[0] == '#' || line[0] == '%' || line[0] == '/' ) continue;

        // normalize separators: tabs and commas to spaces
        for (char &c : line) if (c == '\t' || c == ',') c = ' ';

        std::istringstream iss(line);
        int u=0, v=0;
        double w = 1.0;
        if (!(iss >> u >> v)) {
            ++unparsable;
            if (line_no <= 20) std::cerr << "Warning: cannot parse line " << line_no << ": " << rawline << "\n";
            continue;
        }
        if (!(iss >> w)) w = 1.0;

        // use API
        graph->ensureNode(u);
        graph->ensureNode(v);
        graph->addEdge(u, v, w);

        ++edges_loaded;
        max_node_id = std::max(max_node_id, std::max(u, v));
    }

    std::cout << "Успешно загружен Facebook граф: " << graph->getNodeCount()
              << " пользователей, " << edges_loaded << " дружеских связей\n";
    std::cout << "Максимальный ID пользователя: " << max_node_id << "\n";
    if (unparsable > 0) std::cout << "Нераспаршенных строк: " << unparsable << "\n";

    return graph;
}

} // namespace Infrastructure