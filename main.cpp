#include <windows.h>
#include <windowsx.h>
#include<bits/stdc++.h>

constexpr int64_t N_STATE=6;
constexpr int64_t N_CHAR=2;

constexpr int64_t tm_fmt=2;
struct Trans{
	int64_t nxt,out,dir;
	Trans():nxt(-1),out(1),dir(1){}
	Trans(int64_t s,int64_t o,int64_t d):nxt(s),out(o),dir(d){}
	bool operator==(const Trans &tr)const{
		return nxt==tr.nxt && out==tr.out && dir==tr.dir;
	}
	friend std::ostream &operator<<(std::ostream &os,const Trans &tr){
		os<<("_1"[tr.out])<<' '<<(tr.dir==1?'r':'l')<<' ';
		if(tr.nxt==-1)os<<"halt";
		else os<<tr.nxt;
		return os;
	}
};

struct TM{
	Trans trans[N_STATE][N_CHAR];
	TM(){}
	bool operator==(const TM &tm)const{
		for(int64_t i=0;i<N_STATE;++i)
		for(int64_t j=0;j<N_CHAR;++j){
			if(!(trans[i][j]==tm.trans[i][j]))return 0;
		}
		return 1;
	}
	TM rev()const{
		TM tm=*this;
		for(int64_t s=0;s<N_STATE;++s){
			for(int64_t i=0;i<N_CHAR;++i){
				tm.trans[s][i].dir*=-1;
			}
		}
		return tm;
	}
	friend std::ostream &operator<<(std::ostream &os,const TM &tm){
		if(tm_fmt==1)
		for(int64_t s=0;s<N_STATE;++s){
			for(int64_t i=0;i<2;++i){
				os<<s<<' '<<("_1"[i])<<"  "<<tm.trans[s][i]<<'\n';
			}
		}
		if(tm_fmt==2)
		for(int64_t s=0;s<N_STATE;++s){
			if(s)os<<'_';
			for(int64_t i=0;i<N_CHAR;++i){
				const Trans &tr=tm.trans[s][i];
				if(tr.nxt==-1)os<<"---";
				else os<<char('0'+tr.out)<<(tr.dir==1?'R':'L')<<char('A'+tr.nxt);
			}
		}
		return os;
	}
	friend std::istream &operator>>(std::istream &is,TM &tm){
		std::string str;
		assert(is>>str);
		if(str.length()<N_STATE*N_CHAR*3+N_STATE-1){
			std::cerr<<"\""<<str<<"\"\n";
		}
		assert(str.length()>=N_STATE*N_CHAR*3+N_STATE-1);
		int64_t p=0;
		for(int64_t i=0;i<N_STATE;++i){
			if(i){
				assert(str[p]=='_');
				++p;
			}
			for(int64_t j=0;j<N_CHAR;++j){
				Trans &tr=tm.trans[i][j];
				char o=str[p++];
				char d=str[p++];
				char s=str[p++];
				if(o=='-'&&d=='-'&&s=='-'){
					tr.nxt=-1;
					tr.dir=1;
					tr.out=1;
				}else{
					tr.nxt=s-'A';
					tr.dir=(d=='R'?1:d=='L'?-1:0);
					tr.out=o-'0';
					assert(0<=tr.nxt&&tr.nxt<N_STATE);
					assert(tr.dir*tr.dir==1);
					assert(0<=tr.out&&tr.out<N_CHAR);
				}
			}
		}
		return is;
	}
};
std::vector<TM> read_file(std::string fn){
	std::vector<TM> ret;
	std::cout<<"read from "<<fn<<'\n';
	std::ifstream ifs(fn);
	for(std::string ln;std::getline(ifs,ln);){
		if(ln.back()=='\r')ln.pop_back();
		if(ln.empty())continue;
		std::stringstream ss(ln);
		TM tm;
		ss>>tm;
		ret.emplace_back(tm);
	}
	std::cout<<"read "<<ret.size()<<" TMs\n";
	return ret;
}
std::vector<TM> read_file_1(std::string fn){
	std::vector<TM> ret;
	std::cout<<"read from "<<fn<<'\n';
	std::ifstream ifs(fn);
	for(std::string ln;std::getline(ifs,ln);){
		if(ln.back()=='\r')ln.pop_back();
		if(ln.empty())continue;
		std::stringstream ss(ln);
		int64_t id=0;
		TM tm;
		assert(ss>>id);
		ss>>tm;
		ret.emplace_back(tm);
	}
	std::cout<<"read "<<ret.size()<<" TMs\n";
	return ret;
}

std::vector<TM> ls=read_file("input.txt");

int64_t cur_tm=0;

namespace Basic{
bool pops1(std::vector<bool> &xs){
	bool c=0;
	if(!xs.empty()){
		c=xs.back();
		xs.pop_back();
	}
	return c;
}
struct Tape{
	std::vector<bool> l,r;
	bool in=0;
	int64_t s=0,pos=0;
	bool step(const TM &tm){
		if(s==-1)return 1;
		Trans tr=tm.trans[s][in];
		s=tr.nxt;
		pos+=tr.dir;
		if(tr.dir==1){
			l.push_back(tr.out);
			in=pops1(r);
		}else{
			r.push_back(tr.out);
			in=pops1(l);
		}
		return 0;
	}
	bool at(int64_t i){
		i-=pos;
		if(!i)return in;
		if(i>0){
			size_t i0=i-1;
			return i0<r.size()?r.at(r.size()-1-i0):false;
		}else{
			size_t i0=-i-1;
			return i0<l.size()?l.at(l.size()-1-i0):false;
		}
	}
	std::vector<std::vector<float>> gen_history(const TM &tm,int64_t maxT,int64_t width,int64_t height,int64_t xpos,int64_t ypos){
		std::vector<std::vector<float>> res(height,std::vector<float>(width,0.f));
		int64_t lm=-1,rm=1;
		int64_t ch=maxT/(1+height);
		{
			Tape t=*this;
			for(int64_t i=1;i<=(ch*height);++i){
				if(t.step(tm))break;
				lm=std::min(lm,t.pos);
				rm=std::max(rm,t.pos);
			}
		}
		int64_t lm_=lm,rm_=rm;
		{
			for(int64_t i=1;i<=ch*height/4*ypos;++i){
				if(step(tm))break;
				lm_=std::min(lm_,pos);
				rm_=std::max(rm_,pos);
			}
		}
		int64_t cw2=ceil((5+std::max(-lm,rm))*1./(1+width/2));
		if(!cw2)cw2=1;
		auto mp=[&](int64_t p)->int64_t{
			return width/8*(4-xpos)+int64_t(floor(p*1./cw2));
		};
		std::vector<float> tmp(width,0.f);
		for(int64_t i=lm_;i<=rm_;++i){
			int64_t x=mp(i);
			if(0<=x&&x<width)tmp.at(x)+=at(i)*(1.f/cw2);
		}
		for(int64_t y=0;y<height;++y){
			int64_t lm0=pos,rm0=pos;
			for(int64_t T=0;T<ch;++T){
				if(s==-1)goto o;
				int64_t p0=mp(pos);
				if(0<=p0&&p0<width){
					float d=(tm.trans[s][in].out-int64_t(in))*(1.f/ch/cw2);
					tmp.at(p0)+=d*(ch-T-0.5f);
					lm0=std::min(lm0,pos);
					rm0=std::max(rm0,pos);
				}
				if(step(tm))goto o;
			}
			for(int64_t i=0;i<width;++i){
				res.at(y).at(i)=tmp.at(i);
			}
			int64_t lm1=std::max<int64_t>(0,mp(lm0));
			int64_t rm1=std::min(width-1,mp(rm0));
			while(mp(lm0-1)==lm1)--lm0;
			while(mp(rm0+1)==rm1)++rm0;
			for(int64_t x=lm1;x<=rm1;++x)tmp.at(x)=0;
			for(int64_t i=lm0;i<=rm0;++i){
				int64_t x=mp(i);
				if(0<=x&&x<width)tmp.at(x)+=at(i)*(1.f/cw2);
			}
		}
		o:;
		return res;
	}
};
}


const int N=960;




struct Canvas{
	HDC hdc,mdc;
	HBRUSH hbr,hbr0,hbr1,hbr2;
	HBRUSH hbr_press,hbr_release;
	HFONT hfont;
	HBITMAP hbmp;
	void init(){
		mdc=CreateCompatibleDC(0);
		hbmp=CreateCompatibleBitmap(hdc,N,N);
		SelectObject(mdc,hbmp);
		hbr=CreateSolidBrush(RGB(0,0,0));
		hbr0=CreateSolidBrush(RGB(0xee,0xee,0xee));
		hbr1=CreateSolidBrush(RGB(0xaa,0,0));
		hbr2=CreateSolidBrush(RGB(0,0,0xaa));
		hbr_press=CreateSolidBrush(RGB(0x00,0xff,0xff));
		hbr_release=CreateSolidBrush(RGB(0x00,0x00,0xff));
		hfont=CreateFont(32,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,"Arial");
		SelectObject(mdc,hfont);
		SetBkMode(mdc,TRANSPARENT);
	}
	void reset(bool fin){
		RECT rc={0,0,N,N};
		FillRect(mdc,&rc,fin?hbr0:hbr);
	}
	void drawPixel(int x,int y,COLORREF color){
		SetPixel(mdc,x,y,color);
	}
	void draw(char *a){
		HBITMAP bmp = CreateBitmap(N,N,1,8*4,(void*)a);
		SelectObject(mdc,bmp);
		DeleteObject(hbmp);
		hbmp=bmp;
	}
	void show(){
		BitBlt(hdc, 0, 0, N, N, mdc, 0, 0, SRCCOPY);
	}
}canvas;
bool repaint=1;
int dir=0,scale_dir=0,maxT=1e6,xpos=0,ypos=0;
int save_type=-1;
std::mutex mtx;
/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		case WM_KEYDOWN:{
			std::lock_guard _(mtx);
			switch(wParam){
				case 'a':
				case 'A':{
					dir=-1;
					repaint=1;
					break;
				}
				case 'd':
				case 'D':{
					dir=+1;
					repaint=1;
					break;
				}
				case 'w':
				case 'W':{
					scale_dir=-1;
					repaint=1;
					break;
				}
				case 's':
				case 'S':{
					scale_dir=+1;
					repaint=1;
					break;
				}
				case 'j':
				case 'J':{
					xpos-=1;
					repaint=1;
					break;
				}
				case 'l':
				case 'L':{
					xpos+=1;
					repaint=1;
					break;
				}
				case 'i':
				case 'I':{
					if(ypos>0)ypos-=1;
					repaint=1;
					break;
				}
				case 'k':
				case 'K':{
					ypos+=1;
					repaint=1;
					break;
				}
				case 'o':
				case 'O':{
					xpos=0;
					ypos=0;
					repaint=1;
					break;
				}
				case '0' ... '9':{
					save_type=wParam-'0';
					break;
				}
			}
			break;
		}
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

static int speed=1;
void delay(int t){
	t/=speed;
	int tt=clock();
	while(double(clock()-tt)/CLOCKS_PER_SEC<t/1000.0);
}
static char a[N*N*4];
DWORD WINAPI update(LPVOID lpParam){
	for(;;){
		{
			std::lock_guard _(mtx);
			if(save_type!=-1){
				std::ofstream ofs("log.txt",std::ios::out|std::ios::app);
				ofs<<ls.at(cur_tm)<<" "<<save_type<<'\n';
				ofs.close();
				save_type=-1;
			}
			if(dir){
				cur_tm=std::max<int64_t>(0,std::min<int64_t>(ls.size()-1,cur_tm+dir));
				dir=0;
			}
			if(scale_dir){
				if(scale_dir==1&&maxT<=(1<<30))maxT=maxT*2;
				if(scale_dir==-1&&maxT/2>N)maxT=maxT/2;
				scale_dir=0;
			}
			if(repaint){
				TM tm=ls.at(cur_tm);
				Basic::Tape t;
				auto h=t.gen_history(tm,maxT,N,N,xpos,ypos);
				for(size_t y=0;y<h.size();++y){
					for(size_t x=0;x<h[y].size();++x){
						int c=floor(std::max(0.f,std::min(1.f,h[y][x]))*255);
						for(int i=0;i<4;++i)a[(y*N+x)*4+i]=c;
					}
				}
				canvas.draw(a);
				canvas.show();
			}
		}
		Sleep(10);
	}
	return 0;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Turing Machine Visulizer",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		N, /* width */
		N, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	canvas.hdc=GetDC(hwnd);
	canvas.init();
	if(1){
		DWORD threadID;
		HANDLE hThread=CreateThread(NULL,0,update,NULL,0,&threadID);
	}

	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}
