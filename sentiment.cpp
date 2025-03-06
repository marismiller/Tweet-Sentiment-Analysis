#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <cctype>       // for std::isalnum
#include <iomanip>      // for std::setprecision
#include <stdexcept>    // for std::runtime_error
#include <string>       // allowed in main for bridging
#include "DSString.h"

// -----------------------------------------------------------------------------
// Data Structures
// -----------------------------------------------------------------------------
struct TrainingTweet {
    int sentiment;      // 0 or 4
    DSString tweetID;
    DSString date;
    DSString query;
    DSString username;
    DSString text;
};

struct TestingTweet {
    DSString tweetID;
    DSString date;
    DSString query;
    DSString username;
    DSString text;
};

// -----------------------------------------------------------------------------
// Helper: Convert DSString -> int using std::stoi on c_str()
// -----------------------------------------------------------------------------
bool toInt(const DSString &ds, int &val) {
    try {
        val = std::stoi(ds.c_str()); 
        return true;
    } catch (...) {
        return false;
    }
}

// Trim leading/trailing whitespace
DSString trim(const DSString &s) {
    if (s.length() == 0) return s;
    // find start
    size_t start = 0;
    while (start < s.length() && std::isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }
    if (start >= s.length()) {
        return DSString();
    }
    // find end
    size_t end = s.length() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) {
        end--;
    }
    return s.substring(start, end - start + 1);
}

// Remove surrounding quotes
DSString removeQuotes(const DSString &s) {
    if (s.length() >= 2 && s[0] == '\"' && s[s.length() - 1] == '\"') {
        return s.substring(1, s.length() - 2);
    }
    return s;
}

// Convert to lowercase, keep only alphanumeric
DSString normalizeWord(const DSString &ds) {
    DSString lower = ds.toLower();
    // build
    char* filtered = new char[lower.length() + 1];
    size_t idx = 0;
    for (size_t i = 0; i < lower.length(); i++) {
        char c = lower[i];
        if (std::isalnum(static_cast<unsigned char>(c))) {
            filtered[idx++] = c;
        }
    }
    filtered[idx] = '\0';
    DSString result(filtered);
    delete[] filtered;
    return result;
}

// Tokenize by whitespace -> vector<DSString>
std::vector<DSString> tokenize(const DSString &ds) {
    std::vector<DSString> words;
    std::stringstream ss(ds.c_str()); // bridging to read words
    std::string token;
    while (ss >> token) {
        DSString dsToken(token.c_str());
        DSString norm = normalizeWord(dsToken);
        if (norm.length() > 0) {
            words.push_back(norm);
        }
    }
    return words;
}

// Custom hash for DSString so we can use unordered_map<DSString,int>
namespace std {
    template <>
    struct hash<DSString> {
        size_t operator()(const DSString &s) const noexcept {
            size_t h = 0;
            for (size_t i = 0; i < s.length(); i++) {
                h = 31 * h + static_cast<unsigned char>(s[i]);
            }
            return h;
        }
    };
    template <>
    struct equal_to<DSString> {
        bool operator()(const DSString &a, const DSString &b) const {
            return a == b;
        }
    };
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    if (argc != 6) {
        throw std::runtime_error("Usage: ./sentiment <train.csv> <test.csv> <test_ground_truth.csv> <results.csv> <accuracy.txt>");
    }

    DSString trainingFile(argv[1]);
    DSString testingFile(argv[2]);
    DSString groundFile(argv[3]);
    DSString resultsFile(argv[4]);
    DSString accuracyFile(argv[5]);

    // 1) Read training data
    std::ifstream fin(trainingFile.c_str());
    if (!fin.is_open()) {
        std::cerr << "Cannot open " << trainingFile << "\n";
        return 1;
    }
    {
        // skip possible header
        std::string header;
        if (std::getline(fin, header)) {
            // discard
        }
    }

    std::vector<TrainingTweet> trainingData;
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string sSent, sID, sDate, sQuery, sUser, sText;
        if (!std::getline(ss, sSent, ',')) continue;
        if (!std::getline(ss, sID, ','))   continue;
        if (!std::getline(ss, sDate, ',')) continue;
        if (!std::getline(ss, sQuery, ','))continue;
        if (!std::getline(ss, sUser, ',')) continue;
        if (!std::getline(ss, sText))      continue;

        DSString dsSent(sSent.c_str());
        DSString dsID(sID.c_str());
        DSString dsDate(sDate.c_str());
        DSString dsQuery(sQuery.c_str());
        DSString dsUser(sUser.c_str());
        DSString dsText(sText.c_str());

        dsSent  = trim(dsSent);
        dsID    = trim(dsID);
        dsDate  = trim(dsDate);
        dsQuery = trim(dsQuery);
        dsUser  = trim(dsUser);
        dsText  = trim(removeQuotes(dsText));

        int sentimentVal = 0;
        if (!toInt(dsSent, sentimentVal)) {
            continue; // skip invalid
        }
        trainingData.push_back({ sentimentVal, dsID, dsDate, dsQuery, dsUser, dsText });
    }
    fin.close();

    // 2) Build lexicon (word -> score)
    std::unordered_map<DSString,int> wordScores;
    for (auto &tw : trainingData) {
        int incr = (tw.sentiment == 4) ? 1 : -1;
        auto tokens = tokenize(tw.text);
        for (auto &wd : tokens) {
            wordScores[wd] += incr;
        }
    }

    // 3) Read testing data
    std::ifstream finTest(testingFile.c_str());
    if (!finTest.is_open()) {
        std::cerr << "Cannot open " << testingFile << "\n";
        return 1;
    }
    {
        std::string header;
        if (std::getline(finTest, header)) {
            // discard
        }
    }

    std::vector<TestingTweet> testData;
    while (std::getline(finTest, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string sID, sDate, sQ, sUser, sTxt;
        if (!std::getline(ss, sID, ','))   continue;
        if (!std::getline(ss, sDate, ',')) continue;
        if (!std::getline(ss, sQ, ','))    continue;
        if (!std::getline(ss, sUser, ',')) continue;
        if (!std::getline(ss, sTxt))       continue;

        DSString dsID(sID.c_str());
        DSString dsDate(sDate.c_str());
        DSString dsQ(sQ.c_str());
        DSString dsUser(sUser.c_str());
        DSString dsTxt(sTxt.c_str());

        dsID   = trim(dsID);
        dsDate = trim(dsDate);
        dsQ    = trim(dsQ);
        dsUser = trim(dsUser);
        dsTxt  = trim(removeQuotes(dsTxt));

        testData.push_back({ dsID, dsDate, dsQ, dsUser, dsTxt });
    }
    finTest.close();

    // 4) Read ground truth
    std::ifstream finGT(groundFile.c_str());
    if (!finGT.is_open()) {
        std::cerr << "Cannot open " << groundFile << "\n";
        return 1;
    }
    {
        std::string header;
        if (std::getline(finGT, header)) {
            // discard
        }
    }

    std::unordered_map<DSString,int> groundTruth;
    while (std::getline(finGT, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string sSent, sID;
        if (!std::getline(ss, sSent, ',')) continue;
        if (!std::getline(ss, sID))        continue;

        DSString dsSent(sSent.c_str());
        DSString dsID(sID.c_str());
        dsSent = trim(dsSent);
        dsID   = trim(dsID);

        int val = 0;
        if (!toInt(dsSent, val)) {
            continue;
        }
        groundTruth[dsID] = val;
    }
    finGT.close();

    // 5) Classify test tweets
    std::ofstream foutRes(resultsFile.c_str());
    if (!foutRes.is_open()) {
        std::cerr << "Cannot open " << resultsFile << "\n";
        return 1;
    }

    std::unordered_map<DSString,int> predictions;
    for (auto &t : testData) {
        auto tokens = tokenize(t.text);
        int sumScore = 0;
        for (auto &tk : tokens) {
            auto it = wordScores.find(tk);
            if (it != wordScores.end()) {
                sumScore += it->second;
            }
        }
        int predicted = (sumScore >= 0) ? 4 : 0;
        foutRes << predicted << ", " << t.tweetID << "\n";
        predictions[t.tweetID] = predicted;
    }
    foutRes.close();

    // 6) Accuracy
    std::ofstream foutAcc(accuracyFile.c_str());
    if (!foutAcc.is_open()) {
        std::cerr << "Cannot open " << accuracyFile << "\n";
        return 1;
    }

    int correctCount = 0;
    int totalCount   = 0;
    std::vector<DSString> misclassified;
    for (auto &p : groundTruth) {
        const DSString &id = p.first;
        int actual = p.second;
        auto pit = predictions.find(id);
        if (pit == predictions.end()) {
            // no prediction
            continue;
        }
        int pred = pit->second;
        if (pred == actual) {
            correctCount++;
        } else {
            // build "pred, actual, id"
            char buffer[64];
            std::sprintf(buffer, "%d, %d, ", pred, actual);
            DSString lineStr(buffer);
            lineStr = lineStr + id;
            misclassified.push_back(lineStr);
        }
        totalCount++;
    }

    double accuracy = 0.0;
    if (totalCount > 0) {
        accuracy = double(correctCount) / totalCount;
    }

    foutAcc << std::fixed << std::setprecision(3) << accuracy << "\n";
    for (auto &mc : misclassified) {
        foutAcc << mc << "\n";
    }
    foutAcc.close();

    return 0;
}
