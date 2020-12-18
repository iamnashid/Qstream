#include <curl/curl.h>
#include <iostream>
#include <mpg123.h>
#include <ao/ao.h>

#define BITS 8

mpg123_handle *mh = NULL;
ao_device *dev = NULL;

std::size_t play_stream(void *buffer, std::size_t size, std::size_t nmemb, void *userp);
void process_stream(int num);
std::string surah_url(int num);
void about();

int main(int argc, char *argv[])
{
    int op=0,surah_number=0;
    std::cout << "\t\t\t QSTREAM | Lite Weight Quran Streaming \n" << std::endl;
    std::cout << "\n 1. Stream A Surah " << std::endl;
    std::cout << " 2. About the program " << std::endl;
    std::cout << " 0. Exit " << std::endl;
    std::cout << " Enter a Option : ";
    std::cin >> op;
    switch(op)
    {
        case 1:
        std::cout << "\n Enter the Surah Number : ";
        std::cin >> surah_number;
            process_stream(surah_number);
            break;
        case 2:
            about();
            break;
        case 0:
        std::cout << "\n Exiting Program " << std::endl;
            exit(0);
            break;
        default:
        std::cout << "\n Invalid Option , Please restart the program " << std::endl;
            break;
    }
    system("pause");
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

void process_stream(int num)
{
    ao_initialize();
    mpg123_init();
    mh = mpg123_new(NULL, NULL);
    mpg123_open_feed(mh);
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, play_stream);
    curl_easy_setopt(curl, CURLOPT_URL, surah_url(num).c_str());
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_close(dev);
    ao_shutdown();
}

std::string surah_url(int num)
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
        std::cout << "\n Invalid surah , please enter correct number " << std::endl;
        exit(0);
    }
    return url;
}

void about()
{
    std::cout << " QSTREAM V1.0 " << std::endl;
    std::cout << " A Lite Weight Quran Recitation Streaming Program " << std::endl;
    std::cout << " Developed by Nashid " << std::endl;
}

