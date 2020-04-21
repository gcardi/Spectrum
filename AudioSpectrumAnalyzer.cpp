//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>

#pragma comment( lib, "libfftw3-3" )

//---------------------------------------------------------------------------
USEFORM("FormMain.cpp", frmMain);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
    try
    {
         Application->Initialize();
         Application->MainFormOnTaskBar = true;
         Application->Title = "Audio Spectrum Analyzer";
         Application->CreateForm(__classid(TfrmMain), &frmMain);
         Application->Run();
         while ( Screen->FormCount ) {
            try {
                delete Screen->Forms[0];
            }
            catch ( ... ) {
            }
         }
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
