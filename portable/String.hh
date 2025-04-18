#pragma once

class String {
public:
    static bool StartsWith(const char *string, const char *prefix);
    static bool EndsWith(const char *string, const char *suffix);

private:
    String();
};
