#include <curl/curl.h>
#include <iostream>
#include <mpg123.h>
#include <ao/ao.h>
#include "json.hpp"

#define BITS 8
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

using json = nlohmann::json;

mpg123_handle *mh = NULL;
ao_device *dev = NULL;

std::size_t play_stream(void *buffer, std::size_t size, std::size_t nmemb, void *userp);
static
std::size_t WriteCallBackMemory(char *in, std::size_t size, std::size_t nmemb, std::string *out);
void process_stream(int num, std::string surah_info_url);
std::string audio_url(int num);
void list_surah();
void about();

int main(int argc, char *argv[])
{
    int op=0,surah_number=0;
    std::string surah_url = "https://api.alquran.cloud/v1/surah/";
    std::cout << BLUE << "\n\n\n\t\t\t\t Qstream \n" << std::endl;
    std::cout << GREEN << "\n 1. Stream a Surah " << std::endl;
    std::cout << " 2. List all Surah " << std::endl;
    std::cout << " 3. About the program " << std::endl;
    std::cout << " 0. Exit " << std::endl;
    std::cout << " Enter a Option : ";
    std::cin >> op;
    switch(op)
    {
        case 1:
            std::cout << " Enter the Surah Number : ";
            std::cin >> surah_number;
            surah_url.append(std::to_string(surah_number));
            surah_url.append("/ar.alafasy");
            process_stream(surah_number, surah_url);
            break;
        case 2:
            list_surah();
            break;
        case 3:
            about();
            break;
        case 0:
        std::cout << " Exiting Program " << std::endl;
            exit(0);
            break;
        default:
        std::cout << RED << " Invalid Option , Please restart the program " << std::endl;
            break;
    }
    std::cout << RESET;
    system("pause");
    return 0;
}

std::size_t WriteCallBackMemory(char *in, std::size_t size, std::size_t nmemb, std::string *out)
{
    std::size_t total_size = size * nmemb;
    if(total_size)
    {
        out->append(in, total_size);
        return total_size;
    }
    return 0;
}

std::size_t play_stream(void *buffer, std::size_t size, std::size_t nmemb, void *userp)
{
    int err;
    off_t frame_offset;
    char *audio;
    std::size_t  done;
    ao_sample_format format;
    int channels, encoding;
    long rate;
    mpg123_feed(mh, (const unsigned char*) buffer, size *nmemb);
    do{
        err = mpg123_decode_frame(mh, &frame_offset, (unsigned char**)&audio, &done);
        switch(err)
        {
            case MPG123_NEW_FORMAT:
                mpg123_getformat(mh, &rate, &channels, &encoding);
                format.bits = mpg123_encsize(encoding) * BITS;
                format.rate = rate;
                format.channels = channels;
                format.byte_format = AO_FMT_NATIVE;
                format.matrix = 0;
                dev = ao_open_live(ao_default_driver_id(), &format, NULL);
                break;
            case MPG123_OK:
                ao_play(dev, audio, done);
                break;
            case MPG123_NEED_MORE:
                break;
            default:
                break;
        }
    }while(done > 0);
    return size * nmemb;
}

void process_stream(int num, std::string surah_info_url)
{
    std::string info_buffer;
    CURL *curl_handler;
    curl_handler = curl_easy_init();
    curl_easy_setopt(curl_handler, CURLOPT_URL, surah_info_url.c_str());
    curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, WriteCallBackMemory);
    curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, &info_buffer);
    curl_easy_perform(curl_handler);
    curl_easy_cleanup(curl_handler);
    json stream_info = json::parse(info_buffer);
    std::cout << " Stream Info :- " << std::endl;
    std::cout << " Surah Name : " << stream_info["data"]["englishName"] << std::endl;
    std::cout << " Surah Number : " << stream_info["data"]["number"];
    std::cout << " Number of Ayahs : " << stream_info["data"]["numberOfAyahs"] << std::endl;
    std::cout << " Reciter : " << stream_info["data"]["edition"]["englishName"] << std::endl;
    ao_initialize();
    mpg123_init();
    mh = mpg123_new(NULL, NULL);
    mpg123_open_feed(mh);
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, play_stream);
    curl_easy_setopt(curl, CURLOPT_URL, audio_url(num).c_str());
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_close(dev);
    ao_shutdown();
}

void list_surah()
{
    std::string buffer;
    CURL *curl_handler;
    CURLcode res;
    curl_handler = curl_easy_init();
    if(curl_handler)
    {
        curl_easy_setopt(curl_handler, CURLOPT_URL, "https://api.alquran.cloud/v1/meta");
        curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, WriteCallBackMemory);
        curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, &buffer);
        res = curl_easy_perform(curl_handler);
        if(res == CURLE_OK)
        {
            json metadata = json::parse(buffer);
            std::cout << " List of Surahs :- " << std::endl;
            for(int i=0;i<114;i++)
            {
                std::cout << " Surah Number : " << metadata["data"]["surahs"]["references"][i]["number"] << std::endl;
                std::cout << " Surah Name : " << metadata["data"]["surahs"]["references"][i]["englishName"] << std::endl;
            }
        }
        else
            std::cerr << " curl_easy_perform() failed : " << curl_easy_strerror(res) << std::endl;
    }
    curl_easy_cleanup(curl_handler);
}

std::string audio_url(int num)
{
    std::string url = "https://server8.mp3quran.net/afs/";
    try {
        if (num >= 0 && num <= 9) {
            url.append("00");
            url.append(std::to_string(num));
            url.append(".mp3");
        } else if (num >= 10 && num <= 99) {
            url.append("0");
            url.append(std::to_string(num));
            url.append(".mp3");
        } else if (num >= 100 && num <= 114) {
            url.append(std::to_string(num));
            url.append(".mp3");
        } else {
            throw num;
        }
    } catch (int Error) {
        std::cout << RED << " Invalid surah , please enter correct number " << std::endl;
        exit(0);
    }
    return url;
}

void about()
{
    std::cout << "\033[1;33m QSTREAM V1.0 " << std::endl;
    std::cout << " A Lite Weight Quran Recitation Streaming Program " << std::endl;
    std::cout << " Developed by Nashid " << std::endl;
}

