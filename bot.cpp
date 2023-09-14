/**
 * Created by poko on 11-09-2023
 * Concept and ref python code by Rob Dundas
*/

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <Windows.h>
#include <iostream>
#include <chrono>

#define THRESHOLD 1 //means if there is less than 1 (basically 0) black pixel in img then catch the fish


/**
 * Returns a 30x30 screenshot around mouse cursor
*/
cv::Mat getScreenshot()
{
    POINT mousePos;
    GetCursorPos(&mousePos);

    // box area around the mouse pos
    int captureWidth = 30;
    int captureHeight = 30;
    int X = mousePos.x - (captureWidth/2);
    int Y = mousePos.y - (captureHeight/2);

    // device ctx for entire screen
    HDC ScreenDeviceCtx = GetDC(NULL);

    HDC captureDC = CreateCompatibleDC(ScreenDeviceCtx);

    HBITMAP hBitmap = CreateCompatibleBitmap(ScreenDeviceCtx, captureWidth, captureHeight);

    SelectObject(captureDC, hBitmap);

    BitBlt(captureDC, 0, 0, captureWidth, captureHeight, ScreenDeviceCtx, X, Y, SRCCOPY);

    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(bi));

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = captureWidth;
    bi.biHeight = -captureHeight;
    bi.biPlanes =1;
    bi.biBitCount = 32; //4 byte cuz RGBA
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;

    cv::Mat mat = cv::Mat(captureHeight, captureWidth, CV_8UC4);
    GetDIBits(captureDC, hBitmap, 0, captureHeight, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(captureDC);
    ReleaseDC(NULL, ScreenDeviceCtx);

    return mat;
}

// Small util function that return sum of black pixels in a Mat (image)
int sumBlackPixel(cv::Mat mat)
{
    return (mat.rows * mat.cols) - cv::countNonZero(mat);
}

// Perform right click
void rightClick()
{
    INPUT inp = {0};
    inp.type = INPUT_MOUSE;
    inp.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &inp, sizeof(inp));

    inp.type = INPUT_MOUSE;
    inp.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &inp, sizeof(inp));
}

// I use it for debug
void calcFPS()
{
    static auto startTime = std::chrono::steady_clock::now();
    static int fps = 0;

    auto currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();

    if(elapsedTime >= 1)
    {
        startTime = currentTime;
        std::cout << "FPS: " << fps << std::endl;
        fps=0;
    }
    else{
        fps++;
    }
}



//TODO: create a 'getScreenshot' overload that returns 30x30 screenshot from the center 
//of Game Window (so that we can use mouse even while BOT doing it's work)
//TODO: accept some cmdline args to modify certain behavior of the program (e.g. threshold)


int main()
{
    //Show some general instruction
    std::cout << "[*] Press [6] to On-Off BOT" << std::endl;
    std::cout << "[*] Press [0] (In-Game) to Exit BOT" << std::endl;
    std::cout << "[*] Press [ESC] to Exit Output Window" << std::endl;
    std::cout << "[+] Note: Pressing cross (X) won't exit BOT" << std::endl;
    std::cout << "[+] BOT State: OFF" << std::endl;

    cv::namedWindow("Output", cv::WINDOW_NORMAL);
    
    bool bot = false;
    int key = -1;
    bool firstTime = true;
    uint64_t fish_count = 0;

    while (key != 27 && !GetAsyncKeyState(VK_NUMPAD0))
    {
        if(GetAsyncKeyState(VK_NUMPAD6))
        {
            bot = !bot;
            std::cout << "[+] BOT State: " << (bot?"ON":"OFF") << std::endl;
            firstTime = true;
            fish_count = 0;//reset it every time toggle key pressed 
            Sleep(50);
        }
        
        cv::Mat Snapshot = getScreenshot();
        cv::cvtColor(Snapshot, Snapshot, cv::COLOR_BGR2GRAY);
        cv::imshow("Output", Snapshot);

        if(bot)
        {
            if(firstTime)
            {
                // check if rod is already not casted
                if (sumBlackPixel(Snapshot) < THRESHOLD)
                {
                    rightClick();//cast the rod for the first time
                    Sleep(1900);
                }
                firstTime = false;
                goto waitkey;
            }

            if(sumBlackPixel(Snapshot) < THRESHOLD )
            {
                fish_count++;
                std::cout << "Fish : "<< fish_count << std::endl;

                rightClick();// to get the fish
                Sleep(1300);//wait 1.3 seconds
                rightClick();//cast the rod again
                Sleep(2000);//wait 2 seconds before processing any further
            }
        }
waitkey:
        key = cv::waitKey(35);
        //calcFPS();
    }
    
    cv::destroyAllWindows();
    return 0;
}