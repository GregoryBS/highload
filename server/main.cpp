#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <ctime>   

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string>
#include <map>
#include <vector>

using namespace std;

#define SRV_IP "localhost"
#define SOCK_PORT 80
#define BUFFER_SIZE 10240
#define CONF "/etc/httpd.conf"

#define OK 200
#define FORBIDDEN 403
#define NOT_FOUND 404
#define NOT_ALLOWED 405

class Request {
public:
    Request(char *req) {
        char *elems[4];
        elems[0] = strtok(req, " ");
        if (!elems[0]) {
            ok = false;
            return;
        }
        this->method = elems[0];

        elems[1] = strtok(NULL, " ");
        if (!elems[1]) {
            ok = false;
            return;
        }
        this->url = elems[1];

        elems[2] = strtok(NULL, " \r\n");
        if (!elems[2]) {
            ok = false;
            return;
        }
        this->version = elems[2];

        ok = true;
        return;
    }

    string& get_method() {
        return this->method;
    }

    string& get_url() {
        return this->url;
    }

    string& get_version() {
        return this->version;
    }

    bool get_ok() {
        return this->ok;
    }

private:
    string method, url, version;
    bool ok;
};

class Response {
public:
    Response(string& version, int status) {
        this->status = this->codes[status];
        this->main = (version + " " + this->status + "\r\n" +
                    "Connection: close\r\n" +
                    "Server: " + SRV_IP + "\r\n"
                    );
    }

    string get_string(const string& cont_type, int cont_len, const string& data) {
        auto t = chrono::system_clock::to_time_t(chrono::system_clock::now());
        this-> main += ("Date: " + string(ctime(&t)));
        if (this->status != this->codes[OK])
            return main;

        this->main += ("Content-Type: " + cont_type + "\r\n" +
                       "Content-Length: " + to_string(cont_len) + "\r\n\r\n");
        this->main += data;
        return this->main;
    }

private:
    string status;
    string main;
    map<int, string> codes = {{OK, "200 OK"},
                            {FORBIDDEN, "403 Forbidden"},
                            {NOT_FOUND,"404 Not Found"},
                            {NOT_ALLOWED, "405 Method Not Allowed"}};
};

void read_file(string& readedData, FILE *input) {
    unsigned char byte;
    readedData.clear();
    while (fread(&byte, sizeof(char), 1, input))
        readedData += byte;
}

string url_decode(string& src) {
    string result;
    int sym;
    for (int i = 0; i < src.length(); i++) {
        if (int(src[i]) == 37) {
            sscanf(src.substr(i + 1, 2).c_str(), "%x", &sym);
            result += char(sym);
            i += 2;
        } else {
            result += src[i];
        }
    }
    return result;
}

string parse_url(const string& url, const string& root) {
    string new_url(url);
    int pos = new_url.find("?");
    if (pos != string::npos)
        new_url = new_url.substr(0, pos);

    string path = root + url_decode(new_url);
    return path;
}

string create_response(Request& req, const string& root) {
    map<string, string> types = {
        {"html", "text/html"},
        {"js", "application/javascript"},
        {"css", "text/css"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif", "image/gif"},
        {"swf", "application/x-shockwave-flash"},
        {"txt", "text/plain"},
    };

    if (req.get_method() != "GET" && req.get_method() != "HEAD") {
        Response res(req.get_version(), NOT_ALLOWED);
        return res.get_string("", 0, "");
    }

    string path = parse_url(req.get_url(), root);
    if (path.find("../") != string::npos) {
        Response res(req.get_version(), FORBIDDEN);
        return res.get_string("", 0, "");
    }

    bool isDir = false;
    struct stat s;
    if (stat(path.c_str(), &s) == 0)
    {
        if (s.st_mode & S_IFDIR)
        {
            path += "index.html";
            isDir = true;
        }
    }

    FILE *f = fopen(path.c_str(), "rb");
    if (!f) {
        if (isDir) {
            Response res(req.get_version(), FORBIDDEN);
            return res.get_string("", 0, "");
        } else {
            Response res(req.get_version(), NOT_FOUND);
            return res.get_string("", 0, "");
        }
    }

    string data;
    read_file(data, f);
    fclose(f);

    int dotPos = path.rfind('.');
    string ext = path;
    ext.erase(0, dotPos + 1);

    Response res(req.get_version(), OK);
    string cont_type = types[ext];
    if (cont_type == "")
        cont_type = "text/plain";

    if (req.get_method() == "HEAD")
        return res.get_string(cont_type, data.size(), "");

    return res.get_string(cont_type, data.size(), data);
}


void handler(int sock, const string& addr, const string& rootPath) {
    char buf[BUFFER_SIZE];
    int request = recv(sock, buf, BUFFER_SIZE, 0);
    if (request < 0)
    {
        puts("Recv failed");
        close(sock);
        return;
    }
    else if (request == 0)
    {
        puts("Client disconnected upexpectedly.");
        close(sock);
        return;
    }

    Request req(buf);
    string result = create_response(req, rootPath);
    send(sock, result.c_str(), result.size(), 0);

    close(sock);
}

void worker(int sock, const string& root) {
    int client;
    struct sockaddr_in clientAddr;
    unsigned int clientAddrLen = sizeof(clientAddr);

    while ((client = accept(sock, (struct sockaddr*) &clientAddr, &clientAddrLen))) {
           string ip =  inet_ntoa(clientAddr.sin_addr);
           handler(client, ip, root);
    }
}

class Server {
public:
    Server(int count, const string& folder): forkCount(count), path(folder) {
        struct sockaddr_in serverAddr;
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            perror("Socket error");
            exit(1);
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(SOCK_PORT);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
            close(sock);
            perror("bind");
            exit(1);
        }
        listen(sock, 4);
    }

    ~Server() {
        for (int i = 0; i < forkCount; i++)
            kill(child[i], SIGKILL);
        close(sock);
    }

    void run(void) {
        pid_t pid;
        for (int i = 0; i < forkCount; i++) {
            if ((pid = fork()) == 0) {
                cout << "pid: " << getpid() << endl;
                worker(sock, path);
            } else if (pid < 0) {
                return;
            } else {
                child.push_back(pid);
            }
        }

        int status = 0;
        for (int i = 0; i < forkCount; i++) {
            wait(&status);
            cout << "Fork exited with status: " << status << endl;
        }
    }

private:
    int sock;
    int forkCount;
    string path;
    vector<int> child;
};

int main(void)
{
    string p1, p2, path;
    int cpuLimit = get_nprocs(); 
    ifstream conf(CONF);
    conf >> p1 >> cpuLimit >> p2 >> path;

    Server instance(cpuLimit, path);
    instance.run();
    return 0;
}