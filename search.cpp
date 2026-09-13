#include <iostream>
#include <string>
#include <vector>
using namespace std;

void OpenApp(){
    cout << "App Opened" << endl;
};

int main(){
    
    vector <string> Applist{
    
    "WhatsApp", "Telegram", "Discord", "Spotify", "YouTube", "Netflix", "Instagram", "X", "LinkedIn", "Reddit", 
"Visual Studio Code", "Xcode", "Vim", "MacVim", "Obsidian", "Notion", "TradingView", "Krita", "Applio", "Blender", 
"Figma", "Google Chrome", "Safari", "Brave", "Firefox", "Wireshark", "Nmap", "Metasploit", "Docker", "Terminal", 
"Postman", "Minecraft", "Fortnite", "Steam", "Epic Games", "Slack", "Zoom", "Microsoft Teams", "Skype", "Evernote", 
"Trello", "GitHub Desktop", "Android Studio", "PyCharm", "Sublime Text", "Notepad++", "Audacity", "DaVinci Resolve", "Adobe Photoshop", "Adobe Premiere Pro", 
"Canva", "VLC Media Player", "VirtualBox", "Kaspersky", "Malwarebytes", "1Password", "Google Drive", "iCloud", "Opera", "Microsoft Edge", 
"Tor Browser", "Signal", "Roblox", "Valorant", "League of Legends", "OBS Studio", "ShareX", "CPU-Z", "MSI Afterburner", "VeraCrypt", 
"qBittorrent", "Plex", "LibreOffice", "Microsoft Word", "Microsoft Excel", "GarageBand", "Calculator", "Calendar", "Notes", "Duolingo", 
"Khan Academy", "Coursera", "Codecademy", "Photomath", "Google Earth", "Speedtest", "Termux", "Shazam", "Letterboxd", "Crunchyroll"

};

    string input;

cout << "Search : ";
cin >> input;


for(size_t i=0; i<Applist.size(); ++i){

size_t ptrOFinput=input.length();
size_t ptrOFal= Applist[i].length();

if(ptrOFinput!=ptrOFal){

    continue;

}

size_t rate=0;

const char* ptr = input.c_str();
    const char* pTr = Applist[i].c_str();
    
   for(int n=0; n < ptrOFinput; n++)
    if(pTr[n] == ptr[n]){
        rate++;
    }

if(rate == ptrOFinput){
    OpenApp();
}

    int ptrM = ptrOFinput -1;

if(rate == ptrM){
OpenApp();

    }

}

return 0;

};
