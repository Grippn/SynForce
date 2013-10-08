#include "font.h"
#include "application.h"

font::font()
{
    D3DXFONT_DESC fontDesc;
    fontDesc.CharSet = DEFAULT_CHARSET;
    char faceName[] = ""; //"Times New Roman";
    memcpy(&fontDesc.FaceName, faceName, sizeof(faceName));
    fontDesc.Height = 16;
    fontDesc.Width = 0;
    fontDesc.Italic = false;
    fontDesc.MipLevels = 0;
    fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
    fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    fontDesc.Quality = DEFAULT_QUALITY;
    fontDesc.Weight = FW_DONTCARE;
    CHECK( D3DXCreateFontIndirect(application::device, &fontDesc, mpFont) );
}

void font::OnLostDevice()
{
    mpFont->OnLostDevice();
}

void font::OnResetDevice()
{
    mpFont->OnResetDevice();
}