#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
using namespace std;

struct AppEntry{
    string name;
    string path;
};

static string lowerCopy(string s){
    for(size_t i = 0; i < s.size(); ++i){
        s[i] = (char)tolower((unsigned char)s[i]);
    }
    return s;
}

static string trimCopy(string s){
    while(!s.empty() && isspace((unsigned char)s.front())){
        s.erase(s.begin());
    }
    while(!s.empty() && isspace((unsigned char)s.back())){
        s.pop_back();
    }
    return s;
}

static string compactAlnum(const string& s){
    string out;
    for(size_t i = 0; i < s.size(); ++i){
        unsigned char c = (unsigned char)s[i];
        if(isalnum(c)){
            out += (char)tolower(c);
        }
    }
    return out;
}

static string shellSingleQuote(const string& s){
    string out = "'";
    for(size_t i = 0; i < s.size(); ++i){
        if(s[i] == '\''){
            out += "'\\''";
        }else{
            out += s[i];
        }
    }
    out += "'";
    return out;
}

static bool envFlag(const char* name){
    const char* v = getenv(name);
    if(!v || v[0] == '\0' || v[0] == '0'){
        return false;
    }
    return true;
}

static bool endsWith(const string& s, const string& suf){
    return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
}

static void OpenApp(const AppEntry& app){
    cout << "Opening " << app.name << endl;
    if(envFlag("SENGINE_DRY_RUN")){
        cout << app.path << endl;
        cout << "App Opened" << endl;
        return;
    }
    string cmd = "open " + shellSingleQuote(app.path);
    int rc = system(cmd.c_str());
    if(rc == 0){
        cout << "App Opened" << endl;
    }else{
        cout << "Failed to open" << endl;
    }
}

static int maxInt(int a, int b){
    return a > b ? a : b;
}

static int minInt(int a, int b){
    return a < b ? a : b;
}

static int jaroWinklerPct(const string& s1, const string& s2){
    const int len1 = (int)s1.size();
    const int len2 = (int)s2.size();
    if(len1 == 0 || len2 == 0){
        return 0;
    }
    if(s1 == s2){
        return 100;
    }

    int matchDistance = maxInt(len1, len2) / 2 - 1;
    if(matchDistance < 0){
        matchDistance = 0;
    }

    vector<char> s1Match(len1, 0);
    vector<char> s2Match(len2, 0);

    int matches = 0;
    for(int i = 0; i < len1; ++i){
        int start = maxInt(0, i - matchDistance);
        int end = minInt(i + matchDistance + 1, len2);
        for(int j = start; j < end; ++j){
            if(s2Match[j] || s1[i] != s2[j]){
                continue;
            }
            s1Match[i] = 1;
            s2Match[j] = 1;
            matches++;
            break;
        }
    }
    if(matches == 0){
        return 0;
    }

    int transpositions = 0;
    int k = 0;
    for(int i = 0; i < len1; ++i){
        if(!s1Match[i]){
            continue;
        }
        while(!s2Match[k]){
            k++;
        }
        if(s1[i] != s2[k]){
            transpositions++;
        }
        k++;
    }

    double m = (double)matches;
    double jaro = (m / len1 + m / len2 + (m - transpositions / 2.0) / m) / 3.0;

    int prefix = 0;
    int maxPrefix = minInt(4, minInt(len1, len2));
    for(int i = 0; i < maxPrefix; ++i){
        if(s1[i] != s2[i]){
            break;
        }
        prefix++;
    }

    double winkler = jaro + prefix * 0.1 * (1.0 - jaro);
    int pct = (int)(winkler * 100.0 + 0.5);
    if(pct > 100){
        pct = 100;
    }
    return pct;
}

static bool isFillerToken(const string& w){
    return w == "open" || w == "app" || w == "please" || w == "the"
        || w == "launch" || w == "start" || w == "run" || w == "pls";
}

static bool isGenericWord(const string& w){
    return w == "player" || w == "browser" || w == "media" || w == "games"
        || w == "studio" || w == "desktop" || w == "text" || w == "pro"
        || w == "academy" || w == "box" || w == "view" || w == "band"
        || w == "office" || w == "code" || w == "helper" || w == "installer"
        || w == "uninstaller" || w == "diagnostics" || w == "assistant"
        || w == "monitor" || w == "utility" || w == "setup";
}

static bool similarLen(int a, int b){
    int mn = minInt(a, b);
    int mx = maxInt(a, b);
    return mx - mn <= 3 && mn * 2 >= mx;
}

static bool sameStart(const string& a, const string& b){
    if(a.empty() || b.empty()){
        return false;
    }
    return a[0] == b[0];
}

static bool letterTypoStart(const string& a, const string& b){
    if(a.size() < 5 || b.size() < 5){
        return false;
    }
    if(!isalpha((unsigned char)a[0]) || !isalpha((unsigned char)b[0])){
        return false;
    }
    return similarLen((int)a.size(), (int)b.size());
}

static string normalizeQuery(const string& input){
    string lower = lowerCopy(input);
    vector<string> tokens;
    string token;
    for(size_t i = 0; i <= lower.size(); ++i){
        bool boundary = (i == lower.size()) || isspace((unsigned char)lower[i]);
        if(!boundary){
            token += lower[i];
            continue;
        }
        if(!token.empty()){
            tokens.push_back(token);
            token.clear();
        }
    }
    string out;
    for(size_t i = 0; i < tokens.size(); ++i){
        const string& w = tokens[i];
        bool keepApp = (w == "app") && (
            (i + 1 < tokens.size() && tokens[i + 1] == "store")
            || (i > 0 && tokens[i - 1] == "store")
        );
        if(isFillerToken(w) && !keepApp){
            continue;
        }
        if(!out.empty()){
            out += ' ';
        }
        out += w;
    }
    return out;
}

static int scoreAgainst(const string& app, const string& query, bool& laterGeneric){
    laterGeneric = false;
    if(app == query){
        return 100;
    }
    string ca = compactAlnum(app);
    string cq = compactAlnum(query);
    if(!cq.empty() && ca == cq){
        return 100;
    }

    int best = 0;
    if((sameStart(query, app) || letterTypoStart(query, app))
       && similarLen((int)query.size(), (int)app.size())){
        best = maxInt(best, jaroWinklerPct(query, app));
    }
    if((sameStart(cq, ca) || letterTypoStart(cq, ca))
       && similarLen((int)cq.size(), (int)ca.size())){
        best = maxInt(best, jaroWinklerPct(cq, ca));
    }

    size_t start = 0;
    bool first = true;
    while(start < app.size()){
        size_t space = app.find(' ', start);
        size_t end = space == string::npos ? app.size() : space;
        string word = app.substr(start, end - start);
        if(!word.empty()){
            if(word == query){
                if(first){
                    best = maxInt(best, 99);
                }else if(isGenericWord(word)){
                    if(best < 90){
                        laterGeneric = true;
                        best = 90;
                    }
                }else if(query.size() >= 5){
                    best = maxInt(best, 90);
                }
            }else if(query.size() >= 3 && word.size() > query.size()
                     && word.compare(0, query.size(), query) == 0){
                if(first){
                    best = maxInt(best, 98);
                }else if(!isGenericWord(word)){
                    best = maxInt(best, 92);
                }
            }else if(first && query.size() >= 3
                     && similarLen((int)query.size(), (int)word.size())
                     && query[0] == word[0]){
                best = maxInt(best, jaroWinklerPct(query, word));
            }
        }
        if(space == string::npos){
            break;
        }
        start = space + 1;
        first = false;
    }
    return best;
}

static int minScoreFor(const string& query){
    size_t n = query.size();
    if(n <= 2){
        return 100;
    }
    if(n == 3){
        return 90;
    }
    if(n == 5){
        return 90;
    }
    return 85;
}

static int findAppIndex(const vector<AppEntry>& Applist, const string& target){
    string key = lowerCopy(target);
    for(size_t i = 0; i < Applist.size(); ++i){
        if(lowerCopy(Applist[i].name) == key){
            return (int)i;
        }
    }
    return -1;
}

static bool skipIndexedName(const string& name){
    string n = lowerCopy(name);
    return n.find("url handler") != string::npos
        || n.find("uninstaller") != string::npos
        || n.find("installer") != string::npos
        || n.find("diagnostics") != string::npos;
}

static bool addApp(vector<AppEntry>& apps, const string& path){
    if(!endsWith(path, ".app")){
        return false;
    }
    struct stat st;
    if(stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)){
        return false;
    }
    size_t slash = path.rfind('/');
    string file = slash == string::npos ? path : path.substr(slash + 1);
    string name = file.substr(0, file.size() - 4);
    if(name.empty() || skipIndexedName(name)){
        return false;
    }
    if(findAppIndex(apps, name) >= 0){
        return false;
    }
    AppEntry e;
    e.name = name;
    e.path = path;
    apps.push_back(e);
    return true;
}

static void scanDir(const string& dir, int depth, int maxDepth, vector<AppEntry>& apps){
    if(depth > maxDepth){
        return;
    }
    DIR* d = opendir(dir.c_str());
    if(!d){
        return;
    }
    while(dirent* ent = readdir(d)){
        string n = ent->d_name;
        if(n == "." || n == ".." || n[0] == '.'){
            continue;
        }
        string full = dir;
        full += '/';
        full += n;
        if(endsWith(n, ".app")){
            addApp(apps, full);
            continue;
        }
        struct stat st;
        if(stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode)){
            scanDir(full, depth + 1, maxDepth, apps);
        }
    }
    closedir(d);
}

static void splitColon(const string& s, vector<string>& out){
    string cur;
    for(size_t i = 0; i <= s.size(); ++i){
        if(i == s.size() || s[i] == ':'){
            cur = trimCopy(cur);
            if(!cur.empty()){
                out.push_back(cur);
            }
            cur.clear();
        }else{
            cur += s[i];
        }
    }
}

static void loadApps(vector<AppEntry>& apps){
    vector<string> roots;
    roots.push_back("/Applications");
    const char* home = getenv("HOME");
    if(home && home[0] != '\0'){
        roots.push_back(string(home) + "/Applications");
    }
    roots.push_back("/System/Applications");
    const char* extra = getenv("SENGINE_SCAN");
    if(extra && extra[0] != '\0'){
        splitColon(extra, roots);
    }
    for(size_t i = 0; i < roots.size(); ++i){
        scanDir(roots[i], 0, 2, apps);
    }
    addApp(apps, "/System/Library/CoreServices/Finder.app");
}

struct Hit{
    int idx;
    int score;
    bool laterGeneric;
};

static int readChoice(int n){
    cout << "Choose 1-" << n << " (Enter = 1, q cancels): ";
    string ans;
    if(!getline(cin, ans)){
        if(envFlag("SENGINE_ASSUME_YES")){
            cout << endl;
            return 1;
        }
        cout << endl << "Cancelled" << endl;
        return -1;
    }
    ans = lowerCopy(trimCopy(ans));
    if(ans.empty()){
        return 1;
    }
    if(ans == "q" || ans == "n" || ans == "no"){
        cout << "Cancelled" << endl;
        return -1;
    }
    int v = 0;
    for(size_t i = 0; i < ans.size(); ++i){
        if(!isdigit((unsigned char)ans[i])){
            cout << "Cancelled" << endl;
            return -1;
        }
        v = v * 10 + (ans[i] - '0');
    }
    if(v < 1 || v > n){
        cout << "Cancelled" << endl;
        return -1;
    }
    return v;
}

static void launchHit(const vector<AppEntry>& Applist, const Hit& hit){
    cout << Applist[hit.idx].name << "  (" << hit.score << "%)" << endl;
    OpenApp(Applist[hit.idx]);
}

int main(){
    vector<AppEntry> Applist;
    loadApps(Applist);
    if(Applist.empty()){
        cout << "No apps found" << endl;
        return 1;
    }
    if(envFlag("SENGINE_DEBUG")){
        cout << "Indexed " << Applist.size() << " apps" << endl;
    }

    vector<pair<string, string> > Aliases{
        {"vscode", "Visual Studio Code"},
        {"vs code", "Visual Studio Code"},
        {"vs", "Visual Studio Code"},
        {"code", "Visual Studio Code"},
        {"visual studio", "Visual Studio Code"},
        {"visualstudiocode", "Visual Studio Code"},
        {"studio code", "Visual Studio Code"},
        {"wa", "WhatsApp"},
        {"whatsapp", "WhatsApp"},
        {"dc", "Discord"},
        {"discord", "Discord"},
        {"tg", "Telegram"},
        {"obs", "OBS Studio"},
        {"obs", "OBS"},
        {"obs studio", "OBS"},
        {"vlc", "VLC Media Player"},
        {"vlc", "VLC"},
        {"rblx", "Roblox"},
        {"roblox", "Roblox"},
        {"calc", "Calculator"},
        {"calculator", "Calculator"},
        {"term", "Terminal"},
        {"terminal", "Terminal"},
        {"chrome", "Google Chrome"},
        {"googlechrome", "Google Chrome"},
        {"google chrome", "Google Chrome"},
        {"crome", "Google Chrome"},
        {"chrom", "Google Chrome"},
        {"google", "Google Chrome"},
        {"chrome", "Chrome"},
        {"crome", "Chrome"},
        {"chrom", "Chrome"},
        {"google", "Chrome"},
        {"chrome", "Brave Browser"},
        {"googlechrome", "Brave Browser"},
        {"crome", "Brave Browser"},
        {"chrom", "Brave Browser"},
        {"google", "Brave Browser"},
        {"brave", "Brave Browser"},
        {"firefox", "Firefox"},
        {"iterm", "iTerm"},
        {"iterm2", "iTerm"},
        {"iterm", "iTerm2"},
        {"iterm2", "iTerm2"},
        {"notepad", "Notepad++"},
        {"notepad", "Nextpad++"},
        {"nextpad", "Nextpad++"},
        {"settings", "System Settings"},
        {"system settings", "System Settings"},
        {"preview", "Preview"},
        {"photos", "Photos"},
        {"maps", "Maps"},
        {"books", "Books"},
        {"podcasts", "Podcasts"},
        {"xcode", "Xcode"},
        {"wireshark", "Wireshark"},
        {"spotify", "Spotify"},
        {"tradingview", "TradingView"},
        {"burp", "Burp Suite"},
        {"utm", "UTM"},
        {"anydesk", "AnyDesk"},
        {"voicemod", "Voicemod"},
        {"passwords", "Passwords"},
        {"weather", "Weather"},
        {"clock", "Clock"},
        {"calendar", "Calendar"},
        {"contacts", "Contacts"},
        {"reminders", "Reminders"},
        {"shortcuts", "Shortcuts"},
        {"siri", "Siri"},
        {"tv", "TV"},
        {"news", "News"},
        {"home", "Home"},
        {"textedit", "TextEdit"},
        {"disk utility", "Disk Utility"},
        {"activity", "Activity Monitor"},
        {"activity monitor", "Activity Monitor"},
        {"screenshot", "Screenshot"},
        {"quicktime", "QuickTime Player"},
        {"facetime", "FaceTime"},
        {"findmy", "FindMy"},
        {"find my", "FindMy"},
        {"appstore", "App Store"},
        {"app store", "App Store"},
        {"pages", "Pages"},
        {"numbers", "Numbers"},
        {"keynote", "Keynote"},
        {"mullvad", "Mullvad VPN"},
        {"mullvad browser", "Mullvad Browser"},
        {"grammarly", "Grammarly Desktop"},
        {"coteditor", "CotEditor"},
        {"arduino", "Arduino IDE"},
        {"claude", "Claude"},
        {"obsidian", "Obsidian"},
        {"macvim", "MacVim"},
        {"vim", "MacVim"},
        {"notes", "Notes"},
        {"mail", "Mail"},
        {"music", "Music"},
        {"messages", "Messages"},
        {"safari", "Safari"},
        {"slack", "Slack"},
        {"steam", "Steam"},
        {"finder", "Finder"},
        {"zen", "Zen"},
        {"ps", "Adobe Photoshop"},
        {"photoshop", "Adobe Photoshop"},
        {"ps", "Photoshop"},
        {"idea", "IntelliJ IDEA"},
        {"intellij", "IntelliJ IDEA"},
        {"word", "Microsoft Word"},
        {"excel", "Microsoft Excel"},
        {"teams", "Microsoft Teams"},
        {"edge", "Microsoft Edge"},
        {"drive", "Google Drive"},
        {"earth", "Google Earth"},
        {"premiere", "Adobe Premiere Pro"},
        {"android", "Android Studio"},
        {"pycharm", "PyCharm"},
        {"docker", "Docker"},
        {"postman", "Postman"},
        {"figma", "Figma"},
        {"notion", "Notion"},
        {"zoom", "Zoom"},
        {"signal", "Signal"},
        {"netflix", "Netflix"},
        {"yt", "YouTube"},
        {"youtube", "YouTube"},
        {"ig", "Instagram"},
        {"insta", "Instagram"},
        {"twitter", "X"},
        {"x.com", "X"}
    };

    string input;
    cout << "Search : ";
    getline(cin, input);
    input = trimCopy(input);

    if(input.empty()){
        cout << "No match" << endl;
        return 0;
    }

    string query = normalizeQuery(input);
    if(query.empty()){
        cout << "No match" << endl;
        return 0;
    }

    int aliasIdx = -1;
    int aliasScore = -1;
    for(size_t i = 0; i < Aliases.size(); ++i){
        const string& key = Aliases[i].first;
        int score = -1;
        if(key == query || compactAlnum(key) == compactAlnum(query)){
            score = 100;
        }else if(query.size() >= 5 && similarLen((int)query.size(), (int)key.size())
                 && query[0] == key[0]){
            score = maxInt(jaroWinklerPct(query, key),
                           jaroWinklerPct(compactAlnum(query), compactAlnum(key)));
            if(score < 90){
                score = -1;
            }
        }
        if(score < 0){
            continue;
        }
        int idx = findAppIndex(Applist, Aliases[i].second);
        if(idx < 0){
            continue;
        }
        if(score > aliasScore){
            aliasIdx = idx;
            aliasScore = score;
        }
    }
    if(aliasScore == 100){
        Hit hit;
        hit.idx = aliasIdx;
        hit.score = 100;
        hit.laterGeneric = false;
        launchHit(Applist, hit);
        return 0;
    }

    vector<Hit> hits;
    for(size_t i = 0; i < Applist.size(); ++i){
        string app = lowerCopy(Applist[i].name);
        bool laterGeneric = false;
        int score = scoreAgainst(app, query, laterGeneric);
        if((int)i == aliasIdx && aliasScore > score){
            score = aliasScore;
            laterGeneric = false;
        }
        if(score < minScoreFor(query)){
            continue;
        }
        Hit hit;
        hit.idx = (int)i;
        hit.score = score;
        hit.laterGeneric = laterGeneric;
        hits.push_back(hit);
    }

    bool anySpecific = false;
    for(size_t i = 0; i < hits.size(); ++i){
        if(!hits[i].laterGeneric){
            anySpecific = true;
            break;
        }
    }
    if(!anySpecific){
        hits.clear();
    }

    stable_sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b){
        if(a.score != b.score){
            return a.score > b.score;
        }
        return a.idx < b.idx;
    });

    if(hits.empty()){
        cout << "No match" << endl;
        return 0;
    }

    const int maxShow = 5;
    int shown = (int)hits.size();
    if(shown > maxShow){
        shown = maxShow;
    }

    if(shown == 1){
        launchHit(Applist, hits[0]);
        return 0;
    }

    for(int i = 0; i < shown; ++i){
        cout << (i + 1) << ") " << Applist[hits[i].idx].name
             << "  (" << hits[i].score << "%)" << endl;
    }
    int choice = readChoice(shown);
    if(choice < 0){
        return 0;
    }
    OpenApp(Applist[hits[choice - 1].idx]);
    return 0;
}
