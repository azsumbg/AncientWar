#include "framework.h"
#include "AncientWar.h"
#include <mmsystem.h>
#include "FCheck.h"
#include "errh.h"
#include "D2BMPLOADER.h"
#include "gifresizer.h"
#include "ancient.h"
#include <d2d1.h>
#include <dwrite.h>
#include <vector>
#include <chrono>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "fcheck.lib")
#pragma comment (lib, "errh.lib")
#pragma comment (lib, "d2bmploader.lib")
#pragma comment (lib, "gifresizer.lib")
#pragma comment (lib, "ancient.lib")


constexpr wchar_t bWinClassName[]{ L"AncientGame" };
constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };

constexpr int mNew{ 1001 };
constexpr int mTurbo{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int no_record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int record{ 2003 };

////////////////////////////////////////////////////////////

WNDCLASS bWin{};
HINSTANCE bIns{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HICON Icon{ nullptr };
HCURSOR mainCur{ nullptr };
HCURSOR outCur{ nullptr };
HDC PaintDC{ nullptr };
PAINTSTRUCT bPaint{};
HWND bHwnd{ nullptr };
MSG bMsg{};
BOOL bRet{ 0 };
UINT bTimer{ 0 };

POINT cur_pos{};

D2D1_RECT_F b1Rect{ 20.0f, 0, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 20.0f, 0, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 + 20.0f, 0, scr_width - 50.0f, 50.0f };

bool pause = false;
bool in_client = true;
bool show_help = false;
bool sound = true;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;
bool name_set = false;

wchar_t current_player[16]{ L"ONE TARLYO" };

int level = 1;
int mins = 0;
int secs = 180;
int score = 0;

/////////////////////////////////////////

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* b1Bckg{ nullptr };
ID2D1RadialGradientBrush* b2Bckg{ nullptr };
ID2D1RadialGradientBrush* b3Bckg{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmText{ nullptr };
IDWriteTextFormat* midText{ nullptr };
IDWriteTextFormat* bigText{ nullptr };

ID2D1Bitmap* bmpField[16]{ nullptr };
ID2D1Bitmap* bmpIntro[24]{ nullptr };
ID2D1Bitmap* bmpShot[8]{ nullptr };
ID2D1Bitmap* bmpHeroL[12]{ nullptr };
ID2D1Bitmap* bmpHeroR[12]{ nullptr };
ID2D1Bitmap* bmpEvilMed[19]{ nullptr };
ID2D1Bitmap* bmpEvil1[18]{ nullptr };
ID2D1Bitmap* bmpEvil2[14]{ nullptr };
ID2D1Bitmap* bmpEvil3[8]{ nullptr };
ID2D1Bitmap* bmpRIP = nullptr;
ID2D1Bitmap* bmpPotion = nullptr;

///////////////////////////////////////////////////

dll::RANDiT RandMachine{};

dll::Object Hero = nullptr;
float hero_dest_x = 0;
float hero_dest_y = 0;
bool hero_attacking = false;
float shot_dest_x = 0;
float shot_dest_y = 0;
int evil_killed_counter = 0;

float RIP_x = 0;
float RIP_y = 0;
bool hero_killed = false;

std::vector<dll::Object> vEvils;
std::vector<dll::Object> vShots;
std::vector<dll::PROTON> vPotions;

///////////////////////////////////////////////////

template<typename T>concept CanBeReleased = requires(T var)
{
    var.Release();
};
template<CanBeReleased U>bool ClearHeap(U** var)
{
    if (*var)
    {
        (*var)->Release();
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
    err << std::endl;
    err << L"NEW ERROR LOG !" << std::endl;
    err << what << L" Time when error occurred: " << std::chrono::system_clock::now() << std::endl;
    err.close();
}
void ClearResources()
{
    if (!ClearHeap(&iFactory))LogError(L"Error clearing iFactory !");
    if (!ClearHeap(&Draw))LogError(L"Error clearing Draw !");
    
    if (!ClearHeap(&b1Bckg))LogError(L"Error clearing b1Bckg !");
    if (!ClearHeap(&b2Bckg))LogError(L"Error clearing b2Bckg !");
    if (!ClearHeap(&b3Bckg))LogError(L"Error clearing b3Bckg !");

    if (!ClearHeap(&txtBrush))LogError(L"Error clearing txtBrush !");
    if (!ClearHeap(&hgltBrush))LogError(L"Error clearing hgltBrush !");
    if (!ClearHeap(&inactBrush))LogError(L"Error clearing inactBrush !");

    if (!ClearHeap(&iWriteFactory))LogError(L"Error clearing iWriteFactory !");
    if (!ClearHeap(&nrmText))LogError(L"Error clearing nrmText !");
    if (!ClearHeap(&midText))LogError(L"Error clearing midText !");
    if (!ClearHeap(&bigText))LogError(L"Error clearing bigText !");

    for (int i = 0; i < 16; ++i)if (!ClearHeap(&bmpField[i]))LogError(L"Error clearing bmpField !");
    for (int i = 0; i < 24; ++i)if (!ClearHeap(&bmpIntro[i]))LogError(L"Error clearing bmpIntro !");
    for (int i = 0; i < 8; ++i)if (!ClearHeap(&bmpShot[i]))LogError(L"Error clearing bmpShot !");
    for (int i = 0; i < 12; ++i)if (!ClearHeap(&bmpHeroL[i]))LogError(L"Error clearing bmpHeroL !");
    for (int i = 0; i < 12; ++i)if (!ClearHeap(&bmpHeroR[i]))LogError(L"Error clearing bmpHeroR !");
    for (int i = 0; i < 19; ++i)if (!ClearHeap(&bmpEvilMed[i]))LogError(L"Error clearing bmpEvilMed !");
    for (int i = 0; i < 18; ++i)if (!ClearHeap(&bmpEvil1[i]))LogError(L"Error clearing bmpEvil1 !");
    for (int i = 0; i < 14; ++i)if (!ClearHeap(&bmpEvil2[i]))LogError(L"Error clearing bmpEvil2 !");
    for (int i = 0; i < 8; ++i)if (!ClearHeap(&bmpEvil3[i]))LogError(L"Error clearing bmpEvil3 !");

    if (!ClearHeap(&bmpRIP))LogError(L"Error clearing bmpRIP !");
    if (!ClearHeap(&bmpPotion))LogError(L"Error clearing bmpPotion !");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"КРИТИМНА ГРЕШКА !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    std::remove(tmp_file);
    ClearResources();
    exit(1);
}
void InitGame()
{
    level = 1;
    score = 0;
    evil_killed_counter = 0;
    mins = 0;
    secs = 180;
    wcscpy_s(current_player, L"ONE TARLYO");
    name_set = false;

    ClearHeap(&Hero);
    
    Hero = dll::CreatureFactory(scr_width / 2 - 50.0f, ground - 50.0f, hero_flag);

    if (!vEvils.empty())for (int i = 0; i < vEvils.size(); ++i)ClearHeap(&vEvils[i]);
    vEvils.clear();

    if (!vShots.empty())for (int i = 0; i < vShots.size(); ++i)ClearHeap(&vShots[i]);
    vShots.clear();

    vPotions.clear();
}
BOOL CheckRecord()
{
    if (score < 1)return no_record;

    int result = 0;
    CheckFile(record_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return first_record;
    }
    else
    {
        std::wifstream check(record_file);
        check >> result;
        check.close();
    }

    if (score > result)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return record;
    }

    return no_record;
}
void GameOver()
{
    PlaySound(NULL, NULL, NULL);
    KillTimer(bHwnd, bTimer);



    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void LevelUp(int bonus)
{
    mins = 0;
    secs = 180 + level * 10;
    evil_killed_counter = 0;

    ClearHeap(&Hero);

    Hero = dll::CreatureFactory(scr_width / 2 - 50.0f, ground - 50.0f, hero_flag);

    if (!vEvils.empty())for (int i = 0; i < vEvils.size(); ++i)ClearHeap(&vEvils[i]);
    vEvils.clear();

    if (!vShots.empty())for (int i = 0; i < vShots.size(); ++i)ClearHeap(&vShots[i]);
    vShots.clear();

    vPotions.clear();

    score += bonus;

    if (Draw && bigText && txtBrush)
    {
        int intro_frame = 0;

        Draw->BeginDraw();
        Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
        Draw->DrawTextW(L"НИВОТО ПРЕМИНАТО !", 19, bigText, D2D1::RectF(50.0f, 300.f, scr_width, scr_height), txtBrush);
        Draw->EndDraw();
        if (sound)
        {
            PlaySound(NULL, NULL, NULL);
            PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
            PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
        }
        else Sleep(2500);

        if (bonus > 0)
        {
            mciSendString(L"play .\\res\\snd\\morse.wav", NULL, NULL, NULL);
            int counter = 0;

            while (bonus > 0)
            {
                wchar_t bonus_text[30]{ L"БОНУС ЗА НИВОТО: " };
                wchar_t add[5] = L"\0";
                int txt_size = 0;

                --bonus;
                counter += 10 + level;
                wsprintf(add, L"%d", counter);
                wcscat_s(bonus_text, add);

                for (int i = 0; i < 30; ++i)
                {
                    if (bonus_text[i] != '\0')txt_size++;
                    else break;
                }

                Draw->BeginDraw();
                Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
                Draw->DrawTextW(bonus_text, txt_size, bigText, D2D1::RectF(50.0f, 300.f, scr_width, scr_height), txtBrush);
                Draw->EndDraw();
                Sleep(100);

                if (bonus % 3 == 0)
                {
                    ++intro_frame;
                    if (intro_frame > 23)intro_frame = 0;
                }
            }
        }
        else
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            Draw->DrawTextW(L"НЯМА БОНУС !", 11, bigText, D2D1::RectF(350.0f, 300.f, scr_width, scr_height), txtBrush);
            Draw->EndDraw();
        }

        Sleep(2000);
    }
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(Icon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Ха, ха, ха ! Забрави си името", L"Забраватор", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                wcscpy_s(current_player, L"ONE TARLYO");
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        SetTimer(hwnd, bTimer, 1000, NULL);
        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");
        
        AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
        AppendMenu(bMain, MF_STRING, mTurbo, L"Турбо");
        AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bMain, MF_STRING, mExit, L"Изход");

        AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
        AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
        AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");
 
        SetMenu(hwnd, bBar);
        InitGame();
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, губиш прогреса по тази игра !\n\nНаистина ли излизаш ?", L"Изход !",
            MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_TIMER:
        if (pause)break;
        secs--;
        mins = secs / 60;
        if (secs <= 0)
        {
            Draw->EndDraw();
            LevelUp(evil_killed_counter);
        }
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(20, 20, 20)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                pause = true;
                in_client = false;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                else if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                else if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }
                else
                {
                    if (b1Hglt || b2Hglt || b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }

                SetCursor(outCur);
                return true;
            }
            else
            {
                if (b1Hglt || b2Hglt || b3Hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b1Hglt = false;
                    b2Hglt = false;
                    b3Hglt = false;
                }
            }

            SetCursor(mainCur);
            return true;
        }
        else
        {
            if (in_client)
            {
                pause = false;
                in_client = true;
            }

            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }

            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, губиш прогреса по тази игра !\n\nНаистина ли рестартираш ?", L"Рестарт !",
                MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            pause = false;
            break;

        case mTurbo:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако продължиш, губиш бонуса за нивото !\n\nНаистина ли продължаваш ?", L"Следващо ниво !",
                MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            LevelUp(0);
            pause = false;
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;


        }
        break;

    case WM_RBUTTONDOWN:
        if (Hero)
        {
            hero_dest_x = LOWORD(lParam);
            hero_dest_y = HIWORD(lParam);

            if (hero_dest_x < Hero->start.x)Hero->dir = dirs::left;
            else Hero->dir = dirs::right;
            Hero->Move((float)(level), hero_dest_x, hero_dest_y);
        }
        break;

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) <= 50)
        {
            if (LOWORD(lParam) >= b1Rect.left && (LOWORD(lParam) <= b1Rect.right))
            {
                if (name_set)
                {
                    if(sound)mciSendString(L"play .\\res\\snd\\negative.wav",NULL,NULL,NULL);
                    break;
                }
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
                break;
            }
            if (LOWORD(lParam) >= b2Rect.left && (LOWORD(lParam) <= b2Rect.right))
            {
                mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    sound = true;
                    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }
            }
        }
        else
        {
            if (hero_attacking)break;
            if (Hero)
            {
                hero_attacking = true;
                shot_dest_x = (float)(LOWORD(lParam));
                shot_dest_y = (float)(HIWORD(lParam));
            }
        }
        break;



    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int result = 0;
    CheckFile(Ltmp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream start(Ltmp_file);
        start << L"Game started at: " << std::chrono::system_clock::now();
        start.close();
    }

    int win_x = (int)(GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2));
    if (GetSystemMetrics(SM_CXSCREEN) < win_x + scr_width || GetSystemMetrics(SM_CYSCREEN) < scr_height + 50)ErrExit(eScreen);

    Icon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 256, 256, LR_LOADFROMFILE));
    if (!Icon)ErrExit(eIcon);

    mainCur = LoadCursorFromFile(L".\\res\\main.ani");
    outCur = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCur || !outCur)ErrExit(eCursor);

    bWin.lpszClassName = bWinClassName;
    bWin.hInstance = bIns;
    bWin.lpfnWndProc = &WinProc;
    bWin.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
    bWin.hCursor = mainCur;
    bWin.hIcon = Icon;
    bWin.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWin))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"АНТИЧНИ БИТКИ !", WS_CAPTION | WS_SYSMENU, win_x, 50, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 Factory class !");
            ErrExit(eD2D);
        }

        if (iFactory)
        {
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 HwndRenderTarget !");
                ErrExit(eD2D);
            }
        }

        if (Draw)
        {
            D2D1_GRADIENT_STOP stops[2]{};
            ID2D1GradientStopCollection* Coll = nullptr;

            stops[0].position = 0;
            stops[0].color = D2D1::ColorF(D2D1::ColorF::BlanchedAlmond);
            stops[1].position = 1.0f;
            stops[1].color = D2D1::ColorF(D2D1::ColorF::Chocolate);

            hr = Draw->CreateGradientStopCollection(stops, 2, &Coll);
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 GradientStopCollection !");
                ErrExit(eD2D);
            }
            if (Coll)
            {
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F((b1Rect.right - 
                    b1Rect.left) / 2, 25.0f), D2D1::Point2F(0, 0), 100.0f, 25.0f), Coll, &b1Bckg);
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left + 150.0f,
                    25.0f), D2D1::Point2F(0, 0), 100.0f, 25.0f), Coll, &b2Bckg);
                hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left + 150.0f,
                    25.0f), D2D1::Point2F(0, 0), 100.0f, 25.0f), Coll, &b3Bckg);
                if (hr != S_OK)
                {
                    LogError(L"Error creating D2D1 RadialGradientBrush for buttons background !");
                    ErrExit(eD2D);
                }

                ClearHeap(&Coll);
            }

            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &txtBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &hgltBrush);
            hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateGray), &inactBrush);

            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 SolidColorBrush for text !");
                ErrExit(eD2D);
            }

            for (int i = 0; i < 16; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\field\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpField[i] = Load(name, Draw);

                if (!bmpField[i])
                {
                    LogError(L"Error loading bmpField !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 24; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\intro\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpIntro[i] = Load(name, Draw);

                if (!bmpIntro[i])
                {
                    LogError(L"Error loading bmpIntro !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 8; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\shot\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpShot[i] = Load(name, Draw);

                if (!bmpShot[i])
                {
                    LogError(L"Error loading bmpShot !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 12; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\l\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpHeroL[i] = Load(name, Draw);

                if (!bmpHeroL[i])
                {
                    LogError(L"Error loading bmpHeroL !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 12; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\hero\\r\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpHeroR[i] = Load(name, Draw);

                if (!bmpHeroR[i])
                {
                    LogError(L"Error loading bmpHeroR !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 19; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil_med\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvilMed[i] = Load(name, Draw);

                if (!bmpEvilMed[i])
                {
                    LogError(L"Error loading bmpEvilMed !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 18; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil1\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil1[i] = Load(name, Draw);

                if (!bmpEvil1[i])
                {
                    LogError(L"Error loading bmpEvil1 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 14; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil2\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil2[i] = Load(name, Draw);

                if (!bmpEvil2[i])
                {
                    LogError(L"Error loading bmpEvil2 !");
                    ErrExit(eD2D);
                }
            }
            for (int i = 0; i < 8; ++i)
            {
                wchar_t name[100] = L".\\res\\img\\evil3\\";
                wchar_t add[100] = L"\0";

                wsprintf(add, L"%d", i);
                wcscat_s(name, add);
                wcscat_s(name, L".png");

                bmpEvil3[i] = Load(name, Draw);

                if (!bmpEvil3[i])
                {
                    LogError(L"Error loading bmpEvil3 !");
                    ErrExit(eD2D);
                }
            }

            bmpRIP = Load(L".\\res\\img\\rip.png", Draw);
            bmpPotion = Load(L".\\res\\img\\potion.png", Draw);
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), 
            reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1 WriteFactory !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 18, L"", &nrmText);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 24, L"", &midText);
            hr = iWriteFactory->CreateTextFormat(L"Segoe Print", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                DWRITE_FONT_STRETCH_NORMAL, 64, L"", &bigText);
            
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1 Text Formats !");
                ErrExit(eD2D);
            }
        }
    }

    if (Draw && bigText && txtBrush)
    {
        wchar_t init_text[34] = L"АНТИЧНИ ВОЙНИ !\n\n   dev. Daniel !";
        wchar_t show_text[34] = L"\0";

        int intro_frame = 0;

        mciSendString(L"play .\\res\\snd\\morse.wav", NULL, NULL, NULL);

        for (int i = 0; i < 34; i++)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            if (i % 3 == 0)
            {
                ++intro_frame;
                if (intro_frame > 23)intro_frame = 0;
            }
            show_text[i] = init_text[i];
            Draw->DrawTextW(show_text, i, bigText, D2D1::RectF(70.0f, 200.0f, scr_width, scr_height), txtBrush);
            Draw->EndDraw();
            Sleep(80);
        }
        Sleep(1200);
    }
}

//////////////////////////////////////////////////


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogError(L"Error in Windows hInstance parameter !");
        ErrExit(eClass);
    }

    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }
        if (pause)
        {
            if (show_help)continue;
            Draw->BeginDraw();
            Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateGray));
            if (bigText && txtBrush)
                Draw->DrawTextW(L"ПАУЗА", 6, bigText, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 50.0f, scr_width,
                    scr_height), txtBrush);
            Draw->EndDraw();
            continue;
        }
        static int field_frame_delay = 5;
        static int field_frame = 0;

        ////////////////////////////////////////////////////////////////////////////////

        if (Hero)
        {
            if ((Hero->dir == dirs::right && Hero->end.x < hero_dest_x)
                || (Hero->dir == dirs::left && Hero->start.x > hero_dest_x))
                Hero->Move((float)(level), hero_dest_x, hero_dest_y);
        }

        if (vEvils.size() < 3 + level && RandMachine(0, 20) == 6)
        {
            float ev_x = static_cast<float>(RandMachine(50, (int)(scr_width - 50.0f)));
            float ev_y = static_cast<float>(RandMachine(50, 200));
            
            switch (RandMachine(0, 3))
            {
            case 0:
                vEvils.push_back(dll::CreatureFactory(ev_x, ev_y, evil1_flag));
                break;

            case 1:
                vEvils.push_back(dll::CreatureFactory(ev_x, ev_y, evil2_flag));
                break;

            case 2:
                vEvils.push_back(dll::CreatureFactory(ev_x, ev_y, evil3_flag));
                break;

            case 3:
                vEvils.push_back(dll::CreatureFactory(ev_x, ev_y, evil_med_flag));
                break;
            }
        }

        if (!vEvils.empty() && Hero)
        {
            dll::PROTON_MESH EvilMesh(vEvils.size());

            for (int i = 0; i < vEvils.size(); i++)
            {
                dll::PROTON OneProton{ vEvils[i]->start.x, vEvils[i]->start.y,
                      vEvils[i]->GetWidth(),vEvils[i]->GetHeight() };
                EvilMesh.push_back(OneProton);
            }

            for (int i = 0; i < vEvils.size(); i++)
            {
                dll::PROT_POINT to_where{ vEvils[i]->AINextMove(EvilMesh, Hero->start.x, Hero->start.y) };
                if (vEvils[i]->GetType() == evil_med_flag)
                {
                    for (int k = 0; k < vEvils.size(); ++k)
                    {
                        if (k != i && vEvils[k]->lifes < 50)
                            to_where = vEvils[i]->AINextMove(EvilMesh, vEvils[k]->start.x, vEvils[k]->start.y);
                    }
                }
                
                vEvils[i]->Move((float)(level), to_where.x, to_where.y);
            }    
        
        }

        if (!vShots.empty())
        {
            for (std::vector<dll::Object>::iterator shot = vShots.begin(); shot < vShots.end(); ++shot)
            {
                if (!(*shot)->Move((float)(level), NULL, NULL))
                {
                    (*shot)->Release();
                    vShots.erase(shot);
                    break;
                }
            }
        }

        if (!vEvils.empty() && !vShots.empty())
        {
            bool killed = false;
            
            for (std::vector<dll::Object>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                for (std::vector<dll::Object>::iterator shot = vShots.begin(); shot < vShots.end(); ++shot)
                {
                    if (!((*shot)->start.x > (*evil)->end.x || (*shot)->end.x < (*evil)->start.x
                        || (*shot)->start.y>(*evil)->end.y || (*shot)->end.y < (*evil)->start.y))
                    {
                        (*shot)->Release();
                        vShots.erase(shot);
                        (*evil)->lifes -= 10;
                        if ((*evil)->lifes <= 0)
                        {
                            if (sound)mciSendString(L"play .\\res\\snd\\evilkilled.wav", NULL, NULL, NULL);
                            ++evil_killed_counter;
                            if (RandMachine(0, 8) == 6)vPotions.push_back(dll::PROTON((*evil)->start.x, (*evil)->start.y,
                                32.0f, 32.0f));
                            (*evil)->Release();
                            vEvils.erase(evil);
                            score += 5 * level;
                            killed = true;
                        }
                        break;
                    }
                }

                if (killed)break;
            }   
        }

        if (!vEvils.empty() && Hero)
        {
            for (std::vector<dll::Object>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                if (!((*evil)->start.x > Hero->end.x || (*evil)->end.x < Hero->start.x
                    || (*evil)->start.y>Hero->end.y || (*evil)->end.y < Hero->start.y))
                {
                    int damage = (*evil)->Attack();
                    Hero->lifes -= damage;
                    if (sound && damage > 0)mciSendString(L"play .\\res\\snd\\hurt.wav", NULL, NULL, NULL);
                    if (Hero->lifes <= 0)
                    {
                        RIP_x = Hero->start.x;
                        RIP_y = Hero->start.y;
                        hero_killed = true;
                        ClearHeap(&Hero);
                        break;
                    }
                }
            }
        }

        if (!vPotions.empty() && Hero)
        {
            for (std::vector<dll::PROTON>::iterator pot = vPotions.begin(); pot < vPotions.end(); ++pot)
            {
                if (pot->end.y <= ground)
                {
                    pot->start.y += level / 5.0f;
                    pot->SetEdges();
                    if (!(Hero->start.x > pot->end.x || Hero->end.x < pot->start.x
                        || Hero->start.y > pot->end.y || Hero->end.y < pot->start.y))
                    {
                        if (Hero->lifes + 20 <= 100)Hero->lifes += 20;
                        else score += 20;
                        if (sound)mciSendString(L"play .\\res\\snd\\takeasset.wav", NULL, NULL, NULL);
                        vPotions.erase(pot);
                        break;
                    }
                }
                else
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\break.wav", NULL, NULL, NULL);
                    vPotions.erase(pot);
                    break;
                }

            }
        }

        // DRAW THINGS **************************************************

        Draw->BeginDraw();

        if (b1Bckg && b2Bckg && b3Bckg && nrmText && txtBrush && hgltBrush && inactBrush)
        {
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), inactBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 10.0f, 10.0f), b1Bckg);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 10.0f, 10.0f), b2Bckg);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 10.0f, 10.0f), b3Bckg);

            if (name_set)
                Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF(b1Rect.left + 20.0f, b1Rect.top + 10.0f,
                    b1Rect.right, b1Rect.bottom), inactBrush);
            else
            {
                if (b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF(b1Rect.left + 20.0f, b1Rect.top + 10.0f,
                    b1Rect.right, b1Rect.bottom), hgltBrush);
                else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF(b1Rect.left + 20.0f, b1Rect.top + 10.0f,
                    b1Rect.right, b1Rect.bottom), txtBrush);
            }
            if (b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF(b2Rect.left + 20.0f, b2Rect.top + 10.0f,
                b2Rect.right, b2Rect.bottom), hgltBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF(b2Rect.left + 20.0f, b2Rect.top + 10.0f,
                b2Rect.right, b2Rect.bottom), txtBrush);
            if (b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, D2D1::RectF(b3Rect.left + 20.0f, b3Rect.top + 10.0f,
                b3Rect.right, b3Rect.bottom), hgltBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, D2D1::RectF(b3Rect.left + 20.0f, b3Rect.top + 10.0f,
                b3Rect.right, b3Rect.bottom), txtBrush);
        }

        Draw->DrawBitmap(bmpField[field_frame], D2D1::RectF(0, 50.0f, scr_width, scr_height));

        --field_frame_delay;
        if (field_frame_delay <= 0)
        {
            field_frame_delay = 5;
            ++field_frame;
            if (field_frame > 15)field_frame = 0;
        }
        ///////////////////////////////////////////////////////////////////

        // DRAW HERO ********************************

        if (hero_killed)
        {
            PlaySound(NULL, NULL, NULL);
            Draw->DrawBitmap(bmpRIP, D2D1::RectF(RIP_x, RIP_y, RIP_x + 43.0f, RIP_y + 50.0f));
            Draw->EndDraw();
            if (sound)PlaySound(L".\\res\\snd\\killed.wav", NULL, SND_SYNC);
            else Sleep(3000);
            GameOver();
        }
        
        if (Hero)
        {
            int curr_frame = Hero->GetFrame();

            if (Hero->dir == dirs::right)
                Draw->DrawBitmap(bmpHeroR[curr_frame], Resizer(bmpHeroR[curr_frame], Hero->start.x, Hero->start.y));
            else 
                Draw->DrawBitmap(bmpHeroL[curr_frame], Resizer(bmpHeroL[curr_frame], Hero->start.x, Hero->start.y));

            Draw->DrawLine(D2D1::Point2F(Hero->start.x - 5.0f, Hero->end.y + 5.0f),
                D2D1::Point2F(Hero->start.x - 5.0f + Hero->lifes / 3.0f, Hero->end.y + 5.0f), txtBrush, 5.0f);
        }

        if (!vEvils.empty())
        {
            for (int i = 0; i < vEvils.size(); ++i)
            {
                int aframe = vEvils[i]->GetFrame();

                switch (vEvils[i]->GetType())
                {
                case evil1_flag:
                    Draw->DrawBitmap(bmpEvil1[aframe], Resizer(bmpEvil1[aframe], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case evil2_flag:
                    Draw->DrawBitmap(bmpEvil2[aframe], Resizer(bmpEvil2[aframe], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case evil3_flag:
                    Draw->DrawBitmap(bmpEvil3[aframe], Resizer(bmpEvil3[aframe], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                case evil_med_flag:
                    Draw->DrawBitmap(bmpEvilMed[aframe], Resizer(bmpEvilMed[aframe], vEvils[i]->start.x, vEvils[i]->start.y));
                    break;

                }

                Draw->DrawLine(D2D1::Point2F(vEvils[i]->start.x - 5.0f, vEvils[i]->end.y + 5.0f),
                    D2D1::Point2F(vEvils[i]->start.x - 5.0f + vEvils[i]->lifes / 3.0f, vEvils[i]->end.y + 5.0f),txtBrush, 5.0f);
            }
        }

        if (Hero && hero_attacking)
        {
            if (Hero->Attack() > 0)
            {
                hero_attacking = false;

                if (Hero->dir == dirs::left)
                    vShots.push_back(dll::ShotFactory(Hero->start.x, Hero->start.y, shot_dest_x, shot_dest_y));
                else
                    vShots.push_back(dll::ShotFactory(Hero->end.x, Hero->start.y, shot_dest_x, shot_dest_y));
            }
        }

        if (!vShots.empty())
        {
            for (std::vector<dll::Object>::iterator shot = vShots.begin(); shot < vShots.end(); ++shot)
            {
                int aframe = (*shot)->GetFrame();

                Draw->DrawBitmap(bmpShot[aframe], Resizer(bmpShot[aframe], (*shot)->start.x, (*shot)->start.y));
            }
        }

        if (!vPotions.empty())
            for (int i = 0; i < vPotions.size(); ++i)
                Draw->DrawBitmap(bmpPotion, D2D1::RectF(vPotions[i].start.x, vPotions[i].start.y,
                    vPotions[i].end.x, vPotions[i].end.y));

        wchar_t status_txt[150] = L"\0";
        wchar_t add[5] = L"\0";
        int stat_size = 0;

        wcscpy_s(status_txt, current_player);
        
        wcscat_s(status_txt, L", ниво: ");
        wsprintf(add, L"%d", level);
        wcscat_s(status_txt, add);
        
        wcscat_s(status_txt, L", резултат: ");
        wsprintf(add, L"%d", score);
        wcscat_s(status_txt, add);

        wcscat_s(status_txt, L", оставащо време: 0");
        wsprintf(add, L"%d", mins);
        wcscat_s(status_txt, add);
        wcscat_s(status_txt, L" : ");
        if (secs - mins * 60 < 10)wcscat_s(status_txt, L"0");
        wsprintf(add, L"%d", secs - mins * 60);
        wcscat_s(status_txt, add);

        for (int i = 0; i < 150; ++i)
        {
            if (status_txt[i] != '\0')stat_size++;
            else break;
        }

        if (midText && hgltBrush)
            Draw->DrawTextW(status_txt, stat_size, midText, D2D1::RectF(10.0f, ground + 5.0f, scr_width, scr_height), hgltBrush);


        ////////////////////////////////////////////////////
        
        Draw->EndDraw();
    }

    std::remove(tmp_file);
    ClearResources();
    return (int) bMsg.wParam;
}