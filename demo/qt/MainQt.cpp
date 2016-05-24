﻿////////////////////////////////////////////////////////////////////////////////
//
// Game GUI - QT Example
//
// (C) by Sven Forstmann in 2015
//
// License : MIT
// http://opensource.org/licenses/MIT
////////////////////////////////////////////////////////////////////////////////
// Mathlib included from 
// http://sourceforge.net/projects/nebuladevice/
////////////////////////////////////////////////////////////////////////////////

#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QMouseEvent>
#include <QtCore/QTimer.h>
#include <QDebug>

#include "libsqt.h"
#include "gui.h"
#include "fbo.h"

#include <assert.h>
#include <string>

void init_gui()
{
	gui.init( Gui::Flags::CONTEXT_MENU ,
				"../data/gui_global.txt" , 
				"../data/gui_skin.txt");

	qDebug() << "*** gui.init done";

	// Main Window

	Gui::Window w=Gui::Window("Hello World",100,100,350,250);	

	// Add Simple Label to Window

	w.label["l"]=Gui::Label("Label",20,70,100,20);

	// Add Simple Button to Window

	w.button["Ok1"]=Gui::Button("OK",20,100,60,20);
	w.button["Ok1"].callback_pressed=
		[](Gui::Window *window,Gui::Button* control,int index)
		{
			window->close();
		};

	w.button["Ok2"]=Gui::Button("",220,100,100,100);
	w.button["Ok2"].skin=Skin(	"../data/smiley.png",
								"../data/smileybw.png",
								"../data/smiley2.png");

	// Add Simple Combo to Window

	w.combo ["cb"]=Gui::Combo(120,100,60,20);
	w.combo ["cb"].add_item("test");
	w.combo ["cb"].add_item(L"東京");
	w.combo ["cb"].set_selected(1);
	w.combo ["cb"].callback_selected=
		[](Gui::Window *w, Gui::Button* control, int index) // text entered callback example
		{
			if (!w) return; if (!control) return;
			Gui::Combo &c=*(Gui::Combo*)control;
			w->label["l"].text=Gui::String( c.selected ) + "  selected";
			w->label["l"].textcolor=vec4f(1,0,0,1);
		};

	// Add Text Edit to Window

	w.textedit["txt"]=Gui::TextEdit(10,"text",20,150,160,20);
	w.textedit["txt"].callback_text_entered=		
		[](Gui::Window *w,Gui::Button* control,int index) // text entered callback example
		{
			Gui::TextEdit &t=*(Gui::TextEdit*)control;
			w->title.text=t.text;
		};

	// Add Radio

	w.radio["rad"]=Gui::Radio(20,190,20,20);	// first button
	w.radio["rad"].add_item(50,190);			// second button
	w.radio["rad"].add_item(80,190);			// third button
	w.radio["rad"].callback_pressed=
		[](Gui::Window *w,Gui::Button* control,int index)
		{	
			w->x += 10;
		};

	// Add Slider

	w.slider["s1"]=Gui::Slider(	0,200,100, 		/* min,max,default*/	
								20,220,160,20);	/* window x,y,sx,sy */ 
	w.slider["s1"].callback_pressed=
	[](Gui::Window *w,Gui::Button* control,int index)
	{
		Gui::Slider &b=*((Gui::Slider*) control);	
		w->button["Ok1"].text=Gui::String(int(b.val));	
	};

	w.slider["s2"]=Gui::Slider(	0,200,100, 		/* min,max,default*/	
								350,100,20,160,	/* window x,y,sx,sy */
								Gui::Slider::VERTICAL );

	// Add File Menu to Window

	Gui::Menu m=Gui::Menu("File",/* x,y,sx,sy */ 9,39,50,20, /* menuwidth */ 100);
	m.add_item("Load",
		[](Gui::Window *window,Gui::Button* control,int index) // menu button callback
		{
			gui.screen[0].window["filebrowser"]=  // open file dialog
				gui_file_dialog( "Load SaveGame" , "Load" , "Cancel" ,
					/*get_current_dir()*/ "..\\data\\win8",".png",	100,100,

					// file dialog ok button callback
					[](Gui::Window *w,Gui::Button* b,int index) 
					{		
						QMessageBox::information(0, 
							w->textedit["Filename"].text.c_str() , 
							w->label["dir"].text.c_str() ,0);		
						w->close(); 			
					}
			);
		});
	m.add_item("LoadQT",
		[](Gui::Window *window,Gui::Button* control,int index) // menu button callback
		{
			// DANGER : This will crash often! Better not open from here! Just a sample!
			QString fn = QFileDialog::getOpenFileName( (QWidget*)gui.global.ptr["GLWidet"], QString("Open File..."),
                         QString(), QString("HTML-Files (*.htm *.html);;All Files (*)"));
		});

	m.add_item("Close",
		[](Gui::Window *w,Gui::Button* control,int index) // menu button callback
		{
			w->parent->close(); // close window
		});

	m.add_menu("submenu");
	m.window.menu[0].add_item("test1");
	m.window.menu[0].add_item("test2");

	w.menu["menu"]=m;

	// Add new Window to Screen 0 (default)

	gui.screen[0].window["hello1"]=w; // finally put window on the screen (1st copy)

	// Modify and Add new Window to Screen 0 (default)

	w.move(500,100);
	w.resize(400,300);
	w.minsize(150,150);

	gui.screen[0].window["hello2"]=w; // finally put window on the screen (2nd copy)

	// -------------------------------------------------------------------------

	// also use our previous menu as context menu and file menu
	// Note : "context_menu" is reserved for the context menu
	//		  all other id names are common menus on the background

	gui.screen[0].menu["context_menu"]=m;

	gui.screen[0].menu["file"]=m;
	gui.screen[0].menu["file"].y=5;

	// -------------------------------------------------------------------------

	// Add Button to Background

	gui.dialog["sample"]=w; // store for later use in the callback

	gui.screen[0].button["more"]=Gui::Button("More Windows Plz!!",50,50,200,20);
	gui.screen[0].button["more"].callback_pressed=
		[](Gui::Window *w,Gui::Button* control,int index) // menu button callback
		{
			Gui::Window w1,w2;
			w1=gui.dialog["sample"];
			w2=gui.dialog["3D"];

			w1.x= timeGetTime() % (int)gui.screen_resolution_x;
			w1.y= timeGetTime() % (int)gui.screen_resolution_y;
			
			w2.x=(timeGetTime()+500) % (int)gui.screen_resolution_x;
			w2.y=(timeGetTime()+300) % (int)gui.screen_resolution_y;

			gui.screen[0].window.add(w1);
			gui.screen[0].window.add(w2);
		};


	// Add Tabbed Window to Background

	Gui::Tab t=Gui::Tab(350,150,300,250,50,20);;
	t.add_tab("Win1");
	t.add_tab("Win2");
	t.add_tab("Win3");
	t.add_tab("Win4");
	loopi(0,4)
	{
		Gui::Button b=Gui::Button("OK",20+i*10,20,60); t.window[i].button.add(b);
		t.window[i].label["lab"]=Gui::Label("some text",20+i*20,90,100);
		t.window[i].button["test2"]=Gui::Button("OK",100+i*10,20,60);
		t.window[i].button["test3"]=Gui::Button("OK",50+i*10,50,60);
	}
	t.flags=Gui::Tab::MOVABLE; // Make it movable
	gui.screen[0].tab["mytab"]=t;


	// Simple 3D Viewer
	// -------------------------------------------------------------------------

	// Add simple Renderer
	{
		// render callback function
		auto render_func=[](Gui::Window *window,Gui::Button* control,int index)
		{	
			if((!control) || (!window) || window->get_toggled() ) return;

			Gui::Window &w=*((Gui::Window*) window);
			Gui::Button &b=w.button["canvas"];

			// resize button to window client area
			b.x=w.pad_left;
			b.y=w.pad_up;
			b.sx=w.sx-w.pad_left-w.pad_right;
			b.sy=w.sy-w.pad_up-w.pad_down;

			bool draw= (b.hover||b.pressed) && (gui.mouse.button[0] || gui.mouse.button[1] || gui.mouse.wheel!=0);
			if(control!=&b)draw=1;

			printf("*** b.pressed %d\n", b.pressed);
			printf("*** gui.mouse.button[0] %d\n", gui.mouse.button[0]);
			printf("*** gui.mouse.button[1] %d\n", gui.mouse.button[1]);
		
			void *fb=b.var.ptr["fbo"]; if(!fb) return;
			FBO &fbo=*(FBO*)fb;

			if(b.sx!=fbo.width || b.sy!=fbo.height )
			{
				fbo.clear();
				fbo.init(b.sx,b.sy);
				
				draw=1;
			}
			b.skin.tex_normal=b.skin.tex_hover=b.skin.tex_selected=fbo.color_tex;
			if (!draw) return;

			quaternion q(b.var.vec4["rotation"]);
			if(gui.mouse.button[0] & b.pressed)				// rotate by left mouse
			{
				quaternion qx,qy;
				qx.set_rotate_y( (float)gui.mouse.dx/100);
				qy.set_rotate_x(-(float)gui.mouse.dy/100);
				q=qy*qx*q;
				b.var.vec4["rotation"]=vec4f(q.x,q.y,q.z,q.w);
			}
			float z=b.var.number["zoom"];
			if(b.hover)										// zoom by wheel
			{
				z=clamp( z-gui.mouse.wheel*2.f,2.f,120.f );
				b.var.number["zoom"]=z;
				gui.mouse.wheel=0;
			}
			vec4f pos=b.var.vec4["position"];
			if(gui.mouse.button[1] && (b.pressed||b.hover))	// move by middle button
			{
				pos=pos+vec4f((float)gui.mouse.dx*z/40000.0f,(float)gui.mouse.dy*z/40000.0f,0,0);
				b.var.vec4["position"]=pos;
			}

			// render to fbo
			fbo.enable();
			glDisable(GL_TEXTURE_2D);
			glClearColor(0.7,0.7,0.7,1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
			glPerspective(z, (GLfloat)b.sx/(GLfloat)b.sy, 0.01 , 10.0);
			glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
			glTranslatef(pos.x,pos.y,-pos.z); // apply movement
			matrix44 m4(q);
			glMultMatrixf(&m4.m[0][0]);	// apply quaternion rotation

			// draw something simple
			glColor4f(0,0,0,1);
			glBegin(GL_LINE_STRIP);
			loopi(0,101) glVertex3f( sin(float(i)*M_PI*2/50) , cos(float(i)*M_PI*3/50) ,cos(float(i)*M_PI*4/50) );
			glEnd();

			glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();
			fbo.disable();
		};

		w=Gui::Window("3D View",150,150,400,400);	
		w.menu["Menu"]=m;
		w.button["canvas"]=Gui::Button("",20,250);
		w.button["canvas"].skin=Skin("");
		w.button["canvas"].var.vec4["position"]=vec4f(0,0,2.5,0); // vec4
		w.button["canvas"].var.vec4["rotation"]=vec4f(1,0,0,0); // quaternion
		w.button["canvas"].var.number["zoom"]=70; // fov
		w.button["canvas"].var.ptr["fbo"]=0;
		w.button["canvas"].callback_all=render_func;
		w.button["canvas"].callback_init=[](Gui::Window *w,Gui::Button* b,int i) // call before drawing the first time
			{
				if(w)if(b){b->var.ptr["fbo"]=new FBO(100,100);} // 100,100 is just for initialization; will be resized
			};
		w.button["canvas"].callback_exit=[](Gui::Window *w,Gui::Button* b,int i) // called from the button's destructor
			{
				if(w)if(b)if(b->var.ptr["fbo"]){delete(((FBO*)b->var.ptr["fbo"]));}
			};
	}
	gui.dialog["3D"]=w;
	gui.screen[0].window["3D View win1"]=w;
}
////////////////////////////////////////////////////////////////////////////////
void draw_gui()
{
	gui.draw();
}
////////////////////////////////////////////////////////////////////////////////
void exit_gui()
{
	gui.exit();
}
////////////////////////////////////////////////////////////////////////////////
class GLWidget : public QOpenGLWidget {

    Q_OBJECT // must include this if you use Qt signals/slots

public:
	int cursorX();
	int cursorY();

public:
    GLWidget(QWidget *parent = NULL);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void timerEvent(QTimerEvent *);
	void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent* event) ;
	void closeEvent(QCloseEvent *event);
};

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setMouseTracking(true);
	QBasicTimer *timer = new QBasicTimer();
	timer->start(30, this);
}
void GLWidget::closeEvent(QCloseEvent *event)
{
	exit_gui();
	event->accept();
}
void GLWidget::timerEvent(QTimerEvent *)
{
	update();
}
void GLWidget::initializeGL() {

	qDebug() << format();

	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init_gui();

	gui.global.ptr["GLWidet"]=this;

	resize(1000,600);
}

void GLWidget::resizeGL(int x, int y) {

	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, x, y, 0, -1, 1);
	gui.screen_resolution_x=x;
	gui.screen_resolution_y=y;
}

void GLWidget::paintGL() {

	glClearColor(1,1,1,1);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	draw_gui();
}

void GLWidget::wheelEvent(QWheelEvent* event) {

	gui.mouse.wheel_update+=event->delta()>0 ? -1 : 1;
	update() ;
}
void GLWidget::mousePressEvent(QMouseEvent *e) {
	if(e->button() == Qt::LeftButton) gui.mouse.button[0] = 1;
	if(e->button() == Qt::MiddleButton) gui.mouse.button[1] = 1;
	if(e->button() == Qt::RightButton) gui.mouse.button[2] = 1;
	//printf("mp %d 1\n",int(e->button()));
	update() ;
}
void GLWidget::mouseReleaseEvent(QMouseEvent *e) {
	if(e->button() == Qt::LeftButton) gui.mouse.button[0] = 0;
	if(e->button() == Qt::MiddleButton) gui.mouse.button[1] = 0;
	if(e->button() == Qt::RightButton) gui.mouse.button[2] = 0;	
	//printf("mr %d 0\n",int(e->button()));
	update() ;
}
void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    //printf("%d, %d\n", event->x(), event->y());
	gui.mouse.x=event->x();
	gui.mouse.y=event->y();
	update() ;
}

void GLWidget::keyReleaseEvent(QKeyEvent* event) {
	int key=event->key();
	if( key==Qt::Key_Escape ) close();
	if( key==Qt::Key_Backspace)gui.keyb.key[ 8 ]=false; 
	if( key==Qt::Key_Enter || key==Qt::Key_Return)gui.keyb.key[ 13 ]=false; 
	if ( event->key()>=32 )
	if ( event->key()<=127 )
	{
		bool shift= (event->modifiers() & Qt::ShiftModifier)?1:0;
		if (!shift) key=key-uchar('A')+uchar('a');
	}
	if(key<255) gui.keyb.key[key]=false;
	event->ignore();
	update() ;
}

void GLWidget::keyPressEvent(QKeyEvent* event) {
	int key=event->key();
	if( key==Qt::Key_Escape ) close();
	if( key==Qt::Key_Enter || key==Qt::Key_Return)gui.keyb.key[ 13 ]=true; 
	if( key==Qt::Key_Backspace)gui.keyb.key[ 8 ]=true; 
	if ( event->key()>=32 )
	if ( event->key()<=127 )
	{
		bool shift= (event->modifiers() & Qt::ShiftModifier)?1:0;
		if (!shift) key=key-uchar('A')+uchar('a');
	}
	if(key<255) gui.keyb.key[key]=true;
	event->ignore();
	update() ;
}

////////////////////////////////////////////////////////////////////////////////

bool GetCursorPos(POINT *p) { assert(p!=NULL); return 0; }

bool GetWindowPos(POINT *p) { assert(p!=NULL); return 0; }

bool SetCursor(bool show) { return 0; }

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

	QSurfaceFormat surface_format = QSurfaceFormat::defaultFormat();
	surface_format.setAlphaBufferSize( 8 );
	surface_format.setDepthBufferSize( 24 );
	// surface_format.setRedBufferSize( 8 );
	// surface_format.setBlueBufferSize( 8 );
	// surface_format.setGreenBufferSize( 8 );
	// surface_format.setOption( QSurfaceFormat::DebugContext );
	// surface_format.setProfile( QSurfaceFormat::NoProfile );
	// surface_format.setRenderableType( QSurfaceFormat::OpenGLES );
	// surface_format.setSamples( 4 );
	// surface_format.setStencilBufferSize( 8 );
	// surface_format.setSwapBehavior( QSurfaceFormat::DefaultSwapBehavior );
	// surface_format.setSwapInterval( 1 );
	// surface_format.setVersion( 2, 0 );
	QSurfaceFormat::setDefaultFormat( surface_format );

    GLWidget window;
    window.show();
	window.move(100,100);

    return app.exec();
}
////////////////////////////////////////////////////////////////////////////////


#include "MainQT.moc"
