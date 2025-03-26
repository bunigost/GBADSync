#include "dropbox.h"
#include <json-c/json.h>
#include <memory>
#include <iostream>

const std::string CLIENT_ID = "z4n5nrlgoypivuw";

Dropbox::Dropbox(std::string token) : _token(token){
}

std::string get_dropbox_access_token(std::string refreshToken) {
    std::string postFields = "client_id=" + CLIENT_ID + "&grant_type=refresh_token&refresh_token=" + refreshToken;

    Curl _curl;
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Connection: close");
    _curl.setURL(std::string("https://api.dropboxapi.com/oauth2/token?" + postFields));
    _curl.setHeaders(headers);

    std::string httpData;
    _curl.setWriteData(&httpData);
    _curl.setCustomRequestPost();

    _curl.perform();

    auto http_code = _curl.getHTTPCode();

    if (http_code != 200) {
       std::cout << "Token auth error: Received " << http_code << std::endl;
       return "";
    }

    struct json_object *parsed_json;
    struct json_object *access_token;
    parsed_json = json_tokener_parse(httpData.c_str());
    json_object_object_get_ex(parsed_json, "access_token", &access_token);

    std::string access_token_str = json_object_get_string(access_token);
    return access_token_str;
}

void Dropbox::upload(std::map<std::pair<std::string, std::string>, std::vector<std::string>> paths){
    for(auto item : paths){
        for(auto path : item.second){
            printf("Uploading %s\n", (item.first.first + path).c_str());
            FILE *file = fopen((item.first.first + path).c_str(), "rb");
            std::string args("Dropbox-API-Arg: {\"path\":\"/" + item.first.second + path + "\",\"mode\": \"add\",\"mute\": false,\"strict_conflict\": false}");
            std::string auth("Authorization: Bearer " + _token);
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, auth.c_str());
            headers = curl_slist_append(headers, args.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
            headers = curl_slist_append(headers, "Expect:");
            _curl.setURL(std::string("https://content.dropboxapi.com/2/files/upload"));
            _curl.setHeaders(headers);
            _curl.setReadData((void *)file);
            _curl.perform();
            fclose(file);
            printf("\n");
        }
    }
}

std::vector<ListResult> Dropbox::list_folder(std::string path) {
    std::vector<ListResult> paths;

    std::string body("{\"path\":\"" + path + "\"}");
    std::string auth("Authorization: Bearer " + _token);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Connection: close");
    _curl.setURL(std::string("https://api.dropboxapi.com/2/files/list_folder"));
    _curl.setHeaders(headers);
    _curl.setBody(body.c_str());

    std::string httpData;
    _curl.setWriteData(&httpData);
    _curl.perform();

    auto http_code = _curl.getHTTPCode();

    if (http_code != 200) {
       std::cout << "Download error: Received " << http_code << std::endl;
    } else {
        struct json_object *parsed_json;
        struct json_object *entries;
        parsed_json = json_tokener_parse(httpData.c_str());
        json_object_object_get_ex(parsed_json, "entries", &entries);

        auto entries_obj = json_object_get_array(entries);
        int len = array_list_length(entries_obj);
        for (int i = 0; i < len; i++) {
            struct json_object *name;
            struct json_object *path_display;
            struct json_object *server_modified;
            std::string server_modified_str("");
            struct json_object *tag;
            struct json_object *entry = (struct json_object *)(array_list_get_idx(entries_obj, i));
            json_object_object_get_ex(entry, "name", &name);
            json_object_object_get_ex(entry, "path_display", &path_display);
            json_object_object_get_ex(entry, ".tag", &tag);

            if (std::string(json_object_get_string(tag)) == "file") {
                json_object_object_get_ex(entry, "server_modified", &server_modified);
                server_modified_str = std::string(json_object_get_string(server_modified));
            }

            ListResult lr = {
                std::string(json_object_get_string(name)),
                std::string(json_object_get_string(path_display)),
                server_modified_str
            };

            paths.push_back(lr);
        }
    }

    return paths;
}

void Dropbox::download(std::string path, std::string destPath) {
    std::cout << "Downloading " << path << " to " << destPath << std::endl;

    FILE *file = fopen((destPath).c_str(), "wb");
    std::string args("Dropbox-API-Arg: {\"path\":\"" + path + "\"}");
    std::string auth("Authorization: Bearer " + _token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth.c_str());
    headers = curl_slist_append(headers, args.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "Content-Length: 0");
    headers = curl_slist_append(headers, "Connection: close");
    _curl.setURL(std::string("https://content.dropboxapi.com/2/files/download"));
    _curl.setHeaders(headers);

    std::string headerData;
    _curl.setHeaderData(&headerData);
    _curl.setWriteFile(file);

    _curl.perform();

    auto http_code = _curl.getHTTPCode();
    if (http_code != 200) {
       std::cout << "Download error: Received " << http_code << std::endl;
    }

    fclose(file);
}
