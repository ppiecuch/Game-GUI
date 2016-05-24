//#################################################################//
#include "Bmp.h"
#ifdef QT_GUI_LIB
#include <QImage>
#else
#include <IL/devil_cpp_wrapper.hpp>
#endif
//#################################################################//
void init_ilu_lib()
{
#ifndef QT_GUI_LIB
	static bool ini=false;

	if(!ini)
	{
		ilInit();
		iluInit();
		ini=true;
	}
#endif
}
//#################################################################//
Bmp::Bmp()
{
	init_ilu_lib();
	width=height=0;
	data.clear();
}
//#################################################################//
Bmp::Bmp(const char*filename, bool convert32)
{
	init_ilu_lib();
	width=height=0;
	load(filename,convert32);
}
//#################################################################//
Bmp::Bmp(int x,int y,int b,unsigned char*buffer)
{
	width=height=0;
	set(x,y,b,buffer);
}
//#################################################################//
Bmp::~Bmp()
{
}
//#################################################################//
void Bmp::save(const char*filename)
{
	flip();
	printf("saving bmp %dx%dx%d\n",width, height, bpp);
#ifdef QT_GUI_LIB
	if(bpp==24)
		QImage(&data[0], width, height, QImage::Format_RGB888).save(filename);
	else if(bpp==32)
		QImage(&data[0], width, height, QImage::Format_ARGB32).save(filename);
#else
	ILuint imageID;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);
	if(bpp==24) 	ilTexImage(width, height, 1, bpp/8, IL_RGB, IL_UNSIGNED_BYTE, &data[0]); //the third agrument is usually always 1 (its depth. Its never 0 and I can't think of a time when its 2)
	if(bpp==32) 	ilTexImage(width, height, 1, bpp/8, IL_RGBA, IL_UNSIGNED_BYTE, &data[0]); //the third agrument is usually always 1 (its depth. Its never 0 and I can't think of a time when its 2)
	ilEnable(IL_FILE_OVERWRITE);
	ilSaveImage(filename);	
	ilDeleteImages(1, &imageID); // Delete the image name.
#endif
	flip();
}
//#################################################################//
void  Bmp::blur(int count)
{
	int x,y,b,c;
	int bytes=bpp/8;
	for(c=0;c<count;c++)
		for(x=0;x<width-1;x++)
			for(y=0;y<height-1;y++)
				for(b=0;b<bytes;b++)
					data[(y*width+x)*bytes+b]=
					    (	(int)data[((y+0)*width+x+0)*bytes+b]+
					      (int)data[((y+0)*width+x+1)*bytes+b]+
					      (int)data[((y+1)*width+x+0)*bytes+b]+
					      (int)data[((y+1)*width+x+1)*bytes+b] ) /4;

}
//#################################################################//
void Bmp::crop(int x,int y,int x0,int y0)
{
	if(data.size()==0)return;
	
	int i,j;
	int bytes=bpp/8;
	unsigned char* newdata=(unsigned char*)malloc(x*y*bytes);

	if(!newdata) error_stop("Bmp::crop : out of memory");

	memset(newdata,0,x*y*bytes);

	int start=bytes*(x0+y0*width);

	for(i=0;i<y;i++)
		if(i<height)
			for(j=0;j<x*bytes;j++)
				if(j<width*bytes)
					newdata[i*x*bytes+j]=data[i*width*bytes+j+start];
	
	set(x,y,bpp,newdata);
	free(newdata);
}
//#################################################################//
void Bmp::convert_24_32()
{
	if(data.size()==0)return ;

	int x=width, y=height;
	if(x==0)return ;
	if(y==0)return ;

	unsigned char* newdata=(unsigned char*)malloc(x*y*4);
	int bytes=bpp/8;

	loopijk(0,0,0, y,x,bytes)
		newdata[i*x*4+j*4+k]=
			data[i*x*bytes+j*bytes+k];

	loopi(0,x*y) newdata[i*4+3]=0;
	set(x,y,32,newdata);
	free(newdata);
}
//#################################################################//
void Bmp::scale(int x,int y)
{
	if(data.size()==0)return ;
	if(x==0)return ;
	if(y==0)return ;

	int bytes=bpp/8;
	unsigned char* newdata=(unsigned char*)malloc(x*y*bytes);;

	if(!newdata) error_stop("Bmp::scale : out of memory");

	int ofs=0;
	loopijk(0,0,0,y,x,bytes)
	{
		newdata[ofs]=
			data[(	(i*height/y)*width+ j*width/x ) *  bytes + k ];ofs++;
	}
	width=x;
	height=y;
	set(x,y,bpp,newdata);
	free(newdata);
}
//#################################################################//
void Bmp::set(int x,int y,int b,unsigned char*buffer)
{
	width=x;height=y;bpp=b;

	data.resize(width*height*(bpp/8));
	if(buffer)memmove(&data[0],buffer,width*height*(bpp/8));
}
//#################################################################//
static void Convert32bitARGBtoRGBA(QImage &q)
{
    uint32_t count=0, max=(uint32_t)(q.height()*q.width());
    uint32_t* p = (uint32_t*)(q.bits());
    uint32_t n;
    while( count<max )
    {
        n = p[count];   //n = ARGB
        p[count] =  0x00000000 |
                ((n<<8)  & 0x0000ff00) |
                ((n<<8)  & 0x00ff0000) |
                ((n<<8)  & 0xff000000) |
                ((n>>24) & 0x000000ff);
                // p[count] = RGBA
        count++;
    }
}

static void Convert32bitRGBAtoARGB(QImage &q)
{
    uint32_t count=0, max=(uint32_t)(q.height()*q.width());
    uint32_t* p = (uint32_t*)(q.bits());
    uint32_t n;
    while( count<max )
    {
        n = p[count];   //n = RGBA
        p[count] =  0x00000000 |
                ((n>>8)  & 0x000000ff) |
                ((n>>8)  & 0x0000ff00) |
                ((n>>8)  & 0x00ff0000) |
                ((n<<24) & 0xff000000);
                // p[count] = ARGB
        count++;
    }
}

static void convertFromGLImage(QImage &img, int w, int h, bool alpha_format, bool include_alpha) {
  if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
    // OpenGL gives RGBA; Qt wants ARGB
    uint *p = (uint*)img.bits();
    uint *end = p + w*h;
    if (alpha_format && include_alpha) {
      while (p < end) {
        uint a = *p << 24;
        *p = (*p >> 8) | a;
        p++;
      }
    } else {
      // This is an old legacy fix for PowerPC based Macs, which
      // we shouldn't remove
      while (p < end) {
        *p = 0xff000000 | (*p>>8);
        ++p;
      }
    }
  } else {
    // OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
    for (int y = 0; y < h; y++) {
      uint *q = (uint*)img.scanLine(y);
      for (int x=0; x < w; ++x) {
        const uint pixel = *q;
        if (alpha_format && include_alpha) {
          *q = ((pixel << 16) & 0xff0000) | ((pixel >> 16) & 0xff)
              | (pixel & 0xff00ff00);
        } else {
          *q = 0xff000000 | ((pixel << 16) & 0xff0000)
              | ((pixel >> 16) & 0xff) | (pixel & 0x00ff00);
        }
        
        q++;
      }
    }
  }
}

static void convertFromGLImage(QImage &img) {
	convertFromGLImage(img,
		img.width(), img.height(),
		img.format()==QImage::Format_ARGB32_Premultiplied || img.format()==QImage::Format_ARGB32,
		true);
}

void Bmp::load(const char *filename, bool convert32)
{
#ifdef QT_GUI_LIB
	QImage i;

	if(!i.load(filename))
	{
		error_stop("Bmp::load file %s not found\n",filename);
	}

	printf("Bmp::loading %s(%d) : %dx%dx%d \n",filename,i.format(),i.width(),i.height(),i.depth());

	if(i.format()==QImage::Format_ARGB32 || i.format()==QImage::Format_RGB32 || convert32)
		i=i.convertToFormat(QImage::Format_ARGB32); else i=i.convertToFormat(QImage::Format_RGB888);

	printf("Bmp::loaded %s(%d) : %dx%dx%d \n",filename,i.format(),i.width(),i.height(),i.depth());

	convertFromGLImage(i);

	set(i.width(),i.height(),i.depth(),i.bits());
#else
	ilImage i;

	if(!i.Load(filename))
	{
		error_stop("Bmp::load file %s not found\n",filename);
	}
	
	if(i.GetData()==0)
	{
		error_stop("Bmp::load 0 pointer\n");
	}
	
	if(i.Format()==IL_RGBA || convert32) i.Convert(IL_RGBA); else i.Convert(IL_RGB);

	printf("Bmp::loaded %s : %dx%dx%d \n",filename,i.Width(),i.Height(),i.Bpp());

	set(i.Width(),i.Height(),i.Bpp()*8,i.GetData());
#endif
}

//#################################################################//
void Bmp::save_float(const char*filename, float* fdata)
{
	if(fdata==0)fdata=(float*)&this->data[0];
	FILE* fn;
	if ((fn = fopen (filename,"wb")) == NULL)  error_stop("Bmp::save_float");
	fwrite(fdata,1,(bpp/8)*width*height,fn);
	fclose(fn);
}
//#################################################################//
bool Bmp::load_float(const char*filename, float* fdata)
{
	if (!fdata)fdata=(float*)&data[0];

	FILE* fn;
	if ((fn = fopen (filename,"rb")) == NULL) return false;
	fread(fdata,1,(bpp/8)*width*height,fn);
	fclose(fn);
	return true;
}
//#################################################################//