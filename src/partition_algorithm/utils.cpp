//
// Created by 19210 on 2024/9/28.
//


#include <string>
#include <algorithm>
namespace dili {
    std::string bigAddStr(std::string one, std::string two);

    std::string bigSubStr(std::string one, std::string two) {
        if (one[0] == '-') {// one < 0
            one = one.substr(1);
            auto res = bigAddStr(one, two);
            if (res[0] == '-') {
                return res.substr(1);
            } else {
                return '-' + res;
            }
        }

        if (two[0] == '-') {// one > 0 two < 0
            two = two.substr(1);
            return bigAddStr(one, two);
        }
        //one > 0 two >0
        if (one.length() < two.length()) {//one < two
            return '-' + bigSubStr(two, one);
        }
        //one > two
        if (one.length() > two.length()) {
            auto prefix_0 = std::string(one.length() - two.length(), '0');
            two = prefix_0 + two;
        }
        std::reverse(one.begin(), one.end());
        std::reverse(two.begin(), two.end());
        std::string res;
        int carry = 0;
        for (auto i = 0; i < one.length(); ++i) {
            int tmp;
            if (one[i] - carry >= two[i]) {
                tmp = one[i] - carry - two[i];
                carry = 0;
            } else {
                tmp = one[i] - carry - two[i] + 10;
                carry = 1;
            }
            res += (char)(tmp + '0');
        }
        std::reverse(res.begin(), res.end());
        return res;
    }

    std::string bigAddStr(std::string one, std::string two) {
        if (one[0] == '-' && two[0] == '-') {//one < 0 && two < 0
            one = one.substr(1);
            two = two.substr(1);
            return '-' + bigAddStr(one, two);
        }

        if (one[0] == '-') {//one < 0 && two > 0
            one = one.substr(1);
            return bigSubStr(two, one);
        }
        if (two[0] == '-') { //one > 0 && two < 0
            two = two.substr(1);
            return bigSubStr(one, two);
        }
        //one > 0 && two > 0
        if (one.length() < two.length()) {// one < two
            return bigAddStr(two, one);
        }
        //one > two
        if (one.length() > two.length()) {
            auto prefix_0 = std::string(one.length() - two.length(), '0');
            two = prefix_0 + two;
        }
        std::reverse(one.begin(), one.end());
        std::reverse(two.begin(), two.end());
        auto carry = 0;
        std::string res;
        for (auto i = 0; i < one.length(); ++i) {
            auto tmp = one[i] - '0' + two[i] - '0' + carry;
            if (tmp > 10) {
                tmp = tmp % 10;
                carry = 1;
            } else {
                carry = 0;
            }
            res += (char)(tmp + '0');
        }
        if (carry > 0) {
            res += '1';
        }
        std::reverse(res.begin(), res.end());
        return res;
    }
}