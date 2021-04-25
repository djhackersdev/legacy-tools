#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>

using namespace Gdiplus;

static Bitmap *renderString(const wchar_t *str)
{
    Pen black(Color::Black);
    FontFamily font(L"Impact");
    RectF rect(0, 0, 512, 44);
    LinearGradientBrush brush(rect, Color(0xFFFFFFFF), Color(0xFF707070), 
        LinearGradientModeVertical);
    GraphicsPath path;
    StringFormat fmt;
    Rect bounds;
    Matrix trans(1, 0, 0, 1, 0, 0);

    // Create path
    path.AddString(str, -1, &font, 0, 42, PointF(-8, -8), &fmt);
    path.GetBounds(&bounds);

    // Squeeze
    if (bounds.Width > 260) {
        trans.SetElements(260.0f / bounds.Width, 0, 0, 1, 0, 0);
        bounds.Width = 260;
    }

    // Allocate
    Bitmap *bmp = new Bitmap(bounds.Width, bounds.Height);
    Graphics g(bmp);

    // Render path
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTransform(&trans);
    g.FillPath(&brush, &path);
    g.DrawPath(&black, &path);

    return bmp;
}

#define put8(f, v) putc(v, f)

static void put16(FILE *f, SHORT value)
{
    fwrite(&value, 2, 1, f);
}

static void emitTGA(const char *filename, Bitmap *bmp)
{
    Rect rect(0, 0, bmp->GetWidth(), bmp->GetHeight());
    BitmapData data;
    FILE *f;

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s for writing\n", filename);
        exit(-1);
    }

    // Put TGA header
    put8(f, 0); put8(f, 0); put8(f, 2);
    put16(f, 0); put16(f, 0); put8(f, 0);
    put16(f, 0); put16(f, 0); put16(f, bmp->GetWidth()); put16(f, bmp->GetHeight());
    put8(f, 32); put8(f, 0x28);

    // Put pixels
    bmp->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &data);
    fwrite(data.Scan0, 4, rect.Width * rect.Height, f);

    // Clean up
    bmp->UnlockBits(&data);
    fclose(f);
}

int main(int argc, char **argv)
{
    GdiplusStartupInput start;
    ULONG_PTR token;
    Bitmap *bmp;
    wchar_t wbuf[512];
    char filename[512];
    char buf[512];
    char *text;
    char *id;
    FILE *f;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s [infile] [outdir]\n", argv[0]);
        return -1;
    }

    GdiplusStartup(&token, &start, NULL);
    f = fopen(argv[1], "r");

    while (true) {
        // Get line
        fgets(buf, 512, f);
        if (feof(f)) break;

        // Toke
        id = strtok(buf, "|\r\n");
        text = strtok(NULL, "|\r\n");

        // Render string
        printf("Generating %s ...\n", id);
        mbstowcs(wbuf, text, 512);
        bmp = renderString(wbuf);

        // Emit TGA
        sprintf(filename, "%s/%s.tga", argv[2], id);
        emitTGA(filename, bmp);

        // Clean up
        delete bmp;
    }

    return 0;
}
