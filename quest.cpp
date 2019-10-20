/* Quest: by Jon Rafkind
 *
 * Remake of a PC game XQuest which is a remake of an old Mac game.
 * Source is a mess, sorry!
 *
 * email: jon@rafkind.com
 */

#define ALLEGRO_NO_FIX_ALIASES

#include "allegro.h"
#include "trigtable.h"
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

void end_round();
void check_win();

BITMAP * work;
BITMAP * border;
#define screen_x 640
#define screen_y 480
#define super_rad 8
#define zoomer 100
#define max_bullet 200
#define max_crystal 40
#define power_rad 8
#define crystal_rad 7
#define max_mine 45
#define mine_rad 7
#define gate_length super_rad
#define max_explode 1000
#define max_evil 50
#define max_kind 9
#define max_evil_bullet 200
#define max_power 6

#define bounce 5
#define inv 4
#define shoot_fast 3
#define make_3 2
#define laser 1
#define no_mine 0

struct evil_guy {

        int x, y;
        double fx, fy;
        double dx, dy;
        double speed;
        int kind;
        int ang;
        int evil_rad;
        bool alive;

};

struct super_exit {

        int x;
        double fx;
        double dx;
        bool open;

};

struct exploder {

        int x, y;
        int life;
        double dx, dy;
        double fx, fy;
        double speed;
        bool alive;

};

struct super_crystal {

        int x, y;
        bool alive;

};

struct super_mine {

        int x, y;
        bool alive;

};

struct super_disk {

        int x, y;
        int bombs;
        int lives;
        int level;
        long int score;
        bool left_held;
        bool right_held;
        double dx, dy;
        double fx, fy;

};

struct super_power {
        int x, y;
        double dx, dy;
        double fx, fy;
        int ang;
        bool alive;
        int power;
};

struct super_bullet {

        int x, y;
        double dx,dy;
        double fx, fy;
        int life;
        bool alive;

};

struct evil_bullet {

        int x, y;
        double dx,dy;
        double fx, fy;
        bool alive;

};

super_bullet guy_bullet[ max_bullet ];
super_disk guy;
super_crystal crys[ max_crystal ];
super_mine mine[ max_mine ];
exploder expl_r[ max_explode ];
evil_guy evil[ max_evil ];
evil_bullet evil_shot[ max_evil_bullet ];
super_exit gate;
bool power[ max_power ];
int power_time[ max_power ];
super_power s_crys;
volatile int speed_counter;

bool r_click(){if ( mouse_b & 2 ) return true; return false; }
bool l_click(){if ( mouse_b & 1 ) return true; return false; }

char *int2str( long int e ) {

        long int h = e;
        bool sign = false;
        if ( h < 0 ) {
                h = -h;
                sign = true;
        }
        char *mine = new char[100];
        for ( int r = 0; r< 100; r++ )
                mine[ r ] = '0';
        int length = 0;

        if ( sign ) {

                mine[0] = '-';

        }

        while ( h > 0 ) {
                length++;
                int gr = sign;
                for ( int r = gr; r< length+gr; r++ )
                        mine[ length+gr-r ] = mine[ length+gr-r-1 ];

                mine[sign] = (char)( h % 10 + (int)'0' );
                h = h / 10;

        }

        if ( length == 0 )
                length = 1;
        char *rep = new char[ length+sign ];
        for ( int r = 0; r< length+sign; r++ )
                rep[r] = mine[r];
        rep[ length+sign ] = '\0';

        delete[] mine;

        return rep;

}

int rnd( int q ){
	if ( q <= 0 ) return 0;
	return (int)( q * ( (float)rand() / ( (float)RAND_MAX + 1.0 ) ) );
}


int tsqr( int r ) {
        return r*r;
}


int tsqrt( int q ) {

        if ( q <= 0 )
                return 0;

        int min = 0;
        while ( tsqr( min ) < q )
                min+=30;
        while ( tsqr( min ) > q )
                min -= 10;
        while ( tsqr( min ) < q )
                min++;
        return min;

}

/* aprint:
 * Wrapper for textprintf to work in allegro 4.1 and 4.0
 */
void aprint( BITMAP * work, FONT * f, int x, int y, int col, const char * str, ... ){

	va_list a;
	char buf[ 512 ];
	va_start( a, str );
	
	uvszprintf(buf, sizeof(buf), str, a);
	va_end(a);
         
	#if ALLEGRO_SUB_VERSION == 1
		textout_ex( work, font, buf, x, y, col, -1 );
	#else
		textout( work, font, buf, x, y, col );
	#endif
	
}

bool av_dist( int x, int y, int x1, int y1, int min ) {

        if ( abs(x1-x) < min && abs(y1-y) < min )
                return true;
        else
                return false;

}

void died_player(){
	guy.lives--;
}

void init_player() {

        for ( int q = 0; q < max_power; q++ )
                power[q] = false;

        guy.x = 320;
        guy.y = 200;
        guy.fx = 320;
        guy.fy = 200;
        guy.dx = 0;
        guy.dy = 0;
        guy.left_held = false;
}


void init_bullets() {

        for ( int x = 0; x < max_bullet; x++ )
                guy_bullet[x].alive = false;

        for ( int x = 0; x < max_evil_bullet; x++ )
                evil_shot[x].alive = false;

}


void init_exit() {

        gate.x = 320-super_rad*gate_length/2;
        gate.dx = 0;
        if ( rnd(10) == rnd(10) )
                gate.dx = (rnd(200))/100;
        gate.fx = gate.x;
        gate.open = false;

}


void init_crystal() {

        for ( int x = 0; x < max_crystal; x++ )
                crys[x].alive = false;

}


void init_mine() {
        for(int x=0;x<max_mine;x++)mine[x].alive=false;
}


void init_evil() {
        for(int x=0;x<max_evil;x++)evil[x].alive=false;
}


void init_expl() {
        for(int x=0;x<max_explode;x++)expl_r[x].alive=false;
}


void draw_border() {

        for ( int q = 0; q < super_rad-1; q++ ) {

                                                  //up
                line( border, q, q, screen_x-1-q, q, 31-q );

                                                  //down
                line( border, q, screen_y-1-q, screen_x-1-q, screen_y-1-q, 31-q );

                                                  //left
                line( border, q, q, q, screen_y-1-q, 31-q );

                                                  //right
                line( border, screen_x-1-q, q, screen_x-1-q, screen_y-1-q, 31-q );

        }

        for ( int q = 0; q < rnd(100) + 250; q++ )
                putpixel( border, rnd(screen_x-super_rad)+super_rad, rnd(screen_y-super_rad)+super_rad, 31-rnd(5) );

}

void speed_timer(){
	speed_counter++;
}
END_OF_FUNCTION( speed_timer );

void first_init() {
        set_trig();
        allegro_init();
        srand( time(NULL) );
        install_timer();
        install_keyboard();
        set_color_depth( 8 );

        set_gfx_mode( GFX_AUTODETECT_WINDOWED, screen_x, screen_y, 0, 0 );

	#if ALLEGRO_SUB_VERSION == 0
        text_mode(-1);
	#endif
        work = create_bitmap( screen_x, screen_y );
        clear( work );
        border = create_bitmap( screen_x, screen_y );
        clear( border );
        draw_border();
        install_mouse();
        position_mouse( 320, 200 );

        init_player();
        guy.score = 0;                            //this is in the right place
        guy.bombs = 3;
        guy.lives = 20;
        guy.level = -1;
        for ( int x = 0; x < max_power; x++ )
                power_time[x] = 0;
        init_bullets();
        init_crystal();
        init_mine();
        init_expl();
        init_evil();
        init_exit();

	LOCK_VARIABLE( speed_counter );
	LOCK_FUNCTION( (void *)speed_timer );
	install_int( speed_timer, 30 );
	speed_counter = 0;

}


void make_guy_bullet( int x, int y, double sx, double sy ) {

        int q = 0;
        while ( q < max_bullet && guy_bullet[q].alive )
                q++;

        if ( q >= max_bullet )
                return;
        guy_bullet[q].x = x;
        guy_bullet[q].y = y;
        guy_bullet[q].dx = sx;
        guy_bullet[q].dy = sy;
        guy_bullet[q].fx = x;
        guy_bullet[q].fy = y;
        guy_bullet[q].alive = true;
        guy_bullet[q].life = 400;

}


void make_evil_bullet( int x, int y, double sx, double sy ) {

        int q = 0;
        while ( q < max_evil_bullet && evil_shot[q].alive )
                q++;

        if ( q >= max_bullet )
                return;
        evil_shot[q].x = x;
        evil_shot[q].y = y;
        evil_shot[q].dx = sx;
        evil_shot[q].dy = sy;
        evil_shot[q].fx = x;
        evil_shot[q].fy = y;
        evil_shot[q].alive = true;

}


bool too_close( int who, int x, int y ) {

        bool cy = false;
        for ( int q = 0; q < max_crystal; q++ )
                if ( crys[q].alive && q != who)
                        if ( dist( crys[q].x, crys[q].y, x, y ) < crystal_rad*2 )
                                cy = true;
        for ( int q = 0; q < max_mine; q++ )
                if ( mine[q].alive )
                        if ( dist( mine[q].x, mine[q].y, x, y ) < crystal_rad+mine_rad )
                                cy = true;
        return cy;
}


bool too_close_mine( int who, int x, int y ) {

        bool cy = false;

        for ( int q = 0; q < max_mine; q++ )
                if ( mine[q].alive && q != who )
                        if ( dist( mine[q].x, mine[q].y, x, y ) < mine_rad*2 )
                                cy = true;
        if ( dist( x, y, 320, 200 ) < super_rad * 3 )
                cy = true;
        if ( dist( x, y, 320, 10 ) < 30 )
                cy = true;

        return cy;

}


int find_crystal() {

        int g = 0;
        while ( g < max_crystal && crys[g].alive )
                g++;
        return g;

}


void make_mine( int many ) {

        for ( int q = 0; q < many; q++ ) {
                mine[q].alive = true;
                mine[q].x = rnd( screen_x-3*super_rad) + 2*super_rad;
                mine[q].y = rnd( screen_y-3*super_rad) + 2*super_rad;
                int tr = 0;
                while ( too_close_mine( q, mine[q].x, mine[q].y ) && tr < 200) {
                        mine[q].x = rnd(screen_x-3*super_rad)+2*super_rad;
                        mine[q].y = rnd(screen_y-3*super_rad)+2*super_rad;
                        tr++;
                }                                 //while
                if ( tr >= 200 )
                        mine[q].alive = false;
        }                                         //for

}


void make_crystal( int many ) {

        for ( int i = 0; i < many; i++ ) {
                int q = find_crystal();
                if ( q >= max_crystal )
                        return;

                crys[q].alive = true;
                crys[q].x = rnd(screen_x-3*super_rad) + 2*super_rad;
                crys[q].y = rnd(screen_y-3*super_rad) + 2*super_rad;
                int tr = 0;
                while ( too_close( q, crys[q].x, crys[q].y ) && tr < 200) {
                        crys[q].x = rnd(screen_x-3*super_rad)+2*super_rad;
                        crys[q].y = rnd(screen_y-3*super_rad)+2*super_rad;
                        tr++;
                }                                 //while
                if ( tr >= 200 )
                        crys[q].alive = false;

        }                                         //for

}                                                 //make_crystal


void make_exploder( int x, int y, double wx, double wy ) {

        int q = 0;
        while ( q < max_explode && expl_r[q].alive )
                q++;
        if ( q >= max_explode )
                return;
        expl_r[q].alive = true;
        expl_r[q].x = x;
        expl_r[q].y = y;
        expl_r[q].fx = x;
        expl_r[q].fy = y;
        expl_r[q].dx = wx;
        expl_r[q].dy = wy;
        expl_r[q].life = rnd(5) + 17;
        expl_r[q].speed = sqrt( wx*wx+wy*wy );

}


int posneg() {
        int q = rnd(2);
        if ( q == 0 )
                return 1;
        else
                return -1;

}


void make_expl( int ax, int ay ) {

        for ( int x = 0; x < rnd(20) + 50; x++ )
                make_exploder( ax, ay, (double)(rnd(400)) / (double)200 * posneg(),
                        (double)(rnd(400)) / (double)200 * posneg() );

}


void get_crystal( int x, int y ) {

        for ( int q = 0;q < max_crystal; q++ )
        if ( crys[q].alive ) {

                if ( av_dist( x, y, crys[q].x, crys[q].y, super_rad*2) )
                if ( dist( x, y, crys[q].x, crys[q].y ) < (super_rad+crystal_rad) ) {
                        crys[q].alive = false;
                        guy.score += 100;
                }

        }

}


void get_mine( int x, int y ) {

        bool cy = false;
        for ( int q = 0; q < max_mine; q++ )
                if ( mine[q].alive )
                        if ( av_dist( x, y, mine[q].x, mine[q].y, super_rad*2) )
                        if ( dist( x, y, mine[q].x, mine[q].y ) < (super_rad+mine_rad) ) {
                                cy = true;
                mine[q].alive = false;
                make_expl( mine[q].x, mine[q].y );

        }
        if ( cy ) {
                make_expl( guy.x, guy.y );
		died_player();
                init_player();
        }

}


int angl(int q){return((q+360)%360);}

void shoot_3( int x, int y, double dx, double dy ) {

        int ang = gang( x, y, x+(int)(dx*2), y+(int)(dy*2) );
        make_guy_bullet( x, y, dx, dy );
        #define fox 9
        make_guy_bullet( x, y, dx/tcos[ang]*tcos[angl(ang-fox)], dy/tsine[ang]*tsine[angl(ang-fox)] );
        make_guy_bullet( x, y, dx/tcos[ang]*tcos[angl(ang+fox)], dy/tsine[ang]*tsine[angl(ang+fox)] );

}


void move_guy() {

        for ( int x = 0; x < max_power; x++ )
        if ( power_time[x] > 0 ) {
                power_time[x]--;
                power[x] = true;
        }
        else
                power[x] = false;

        int sx, sy;
        get_mouse_mickeys(&sx, &sy);
        guy.dx += (double)sx / (double)25;
        guy.dy += (double)sy / (double)25;
        guy.fx += guy.dx;
        guy.fy += guy.dy;
        guy.x = (int)guy.fx;
        guy.y = (int)guy.fy;
        check_win();
        if ( power[ inv ] ) {

                if ( guy.x < super_rad ) {

                        guy.x = super_rad;
                        guy.fx = super_rad;
                        guy.dx = -guy.dx;

                }
                if ( guy.x > screen_x - super_rad ) {
                        guy.x = screen_x-super_rad;
                        guy.fx = guy.x;
                        guy.dx = -guy.dx;
                }
                if ( guy.y < super_rad ) {

                        guy.y = super_rad;
                        guy.fy = super_rad;
                        guy.dy = -guy.dy;
                }
                if ( guy.y > screen_y - super_rad ) {

                        guy.y = screen_y - super_rad;
                        guy.fy = screen_y - super_rad;
                        guy.dy = -guy.dy;
                }

        }                                         //if power[inv]
        else
        if ( guy.x-super_rad < super_rad || guy.x+super_rad > screen_x - super_rad ||
        guy.y-super_rad < super_rad || guy.y+super_rad > screen_y - super_rad ) {
                make_expl( guy.x, guy.y );
		died_player();
                init_player();
        }

        if ( l_click() && ( !guy.left_held || power[shoot_fast] ) ) {
                guy.left_held = true;
                if ( power[ make_3 ] )
                        shoot_3( guy.x, guy.y, guy.dx*1.5, guy.dy*1.5 );
                else
                        make_guy_bullet( guy.x, guy.y, guy.dx*1.5, guy.dy*1.5 );
        }
        if ( !l_click() )
                guy.left_held = false;
        if ( r_click() && !guy.right_held && guy.bombs > 0 ) {

		guy.bombs--;
                for ( int q = 0; q < max_evil; q++ )
                        if ( evil[q].alive )
                                make_expl( evil[q].x, evil[q].y );
                init_bullets();
                init_evil();
		guy.right_held = true;

        }
        if ( !r_click() )
                guy.right_held = false;

        get_crystal( guy.x, guy.y );
        get_mine( guy.x, guy.y );

}


void draw_guy() {

        if ( power[inv] )
                circle( work, guy.x, guy.y, super_rad, 37+rnd(9) );
        else
                circle( work, guy.x, guy.y, super_rad, 31 );
        line( work, guy.x, guy.y-super_rad, guy.x, guy.y+super_rad, 39 );
        line( work, guy.x-super_rad, guy.y, guy.x+super_rad, guy.y, 39 );

        int x1 = guy.x;
        int y1 = guy.y;
        int x2 = guy.x+(int)( guy.dx * 5 );
        int y2 = guy.y+(int)( guy.dy * 5 );
        int ang = gang( x1, y1, x2, y2 );
        line( work, x1, y1,
                (int)(x1+tcos[ang]*super_rad*3/2), (int)(y1+tsine[ang]*super_rad*3/2),
                32 );

}


void check_q( int & x, int & y, double & wx, double & wy ) {

        if ( x < super_rad )
                x = super_rad;
        if ( x > screen_x-super_rad )
                x = screen_x-super_rad;
        if ( y < super_rad )
                y = super_rad;
        if ( y > screen_y-super_rad )
                y = screen_y-super_rad;

        wx = x;
        wy = y;

}


void move_bullet() {

        for ( int q = 0; q < max_bullet; q++ )
        if ( guy_bullet[q].alive ) {

                guy_bullet[q].fx += guy_bullet[q].dx;
                guy_bullet[q].fy += guy_bullet[q].dy;
                guy_bullet[q].x = (int)guy_bullet[q].fx;
                guy_bullet[q].y = (int)guy_bullet[q].fy;
                if ( guy_bullet[q].x > screen_x-super_rad || guy_bullet[q].x < super_rad ||
                        guy_bullet[q].y > screen_y-super_rad || guy_bullet[q].y < super_rad )
                        if ( !power[ bounce ] )
                                guy_bullet[q].alive = false;
                else {

                        if ( guy_bullet[q].x < super_rad || guy_bullet[q].x > screen_x-super_rad )
                                guy_bullet[q].dx = -guy_bullet[q].dx;
                        if ( guy_bullet[q].y < super_rad || guy_bullet[q].y > screen_y-super_rad )
                                guy_bullet[q].dy = -guy_bullet[q].dy;
                        check_q( guy_bullet[q].x, guy_bullet[q].y, guy_bullet[q].fx, guy_bullet[q].fy );

                }
                guy_bullet[q].life--;
                if ( guy_bullet[q].life <= 0 )
                        guy_bullet[q].alive = false;

        }

}


void move_evil_bullet() {

        for ( int q = 0; q < max_evil_bullet; q++ )
        if ( evil_shot[q].alive ) {

                evil_shot[q].fx += evil_shot[q].dx;
                evil_shot[q].fy += evil_shot[q].dy;
                evil_shot[q].x = (int)evil_shot[q].fx;
                evil_shot[q].y = (int)evil_shot[q].fy;
                if ( evil_shot[q].x > screen_x-super_rad || evil_shot[q].x < super_rad ||
                        evil_shot[q].y > screen_y-super_rad || evil_shot[q].y < super_rad )
                        evil_shot[q].alive = false;
                if ( dist( evil_shot[q].x, evil_shot[q].y, guy.x, guy.y ) <= super_rad ) {
                        make_expl( guy.x, guy.y );
			died_player();
                        init_player();
                        evil_shot[q].alive = false;
                }
        }

}

void draw_bullet() {

        for ( int q = 0; q < max_bullet; q++ )
        if ( guy_bullet[q].alive ) {

                rectfill( work, guy_bullet[q].x-1, guy_bullet[q].y-1,
                        guy_bullet[q].x+1, guy_bullet[q].y+1, 31 );

        }

}


void draw_evil_bullet() {

        for ( int q = 0; q < max_evil_bullet; q++ )
        if ( evil_shot[q].alive ) {

                rectfill( work, evil_shot[q].x-1, evil_shot[q].y-1,
                        evil_shot[q].x+1, evil_shot[q].y+1, 42 );

        }

}


void draw_crystal() {

        for ( int q = 0; q < max_crystal; q++ )
        if ( crys[q].alive ) {

                int sx = crys[q].x;
                int sy = crys[q].y;
                int sl = crystal_rad;

                line( work, sx-sl, sy, sx, sy-sl, 45 );
                line( work, sx, sy-sl, sx+sl, sy, 45 );
                line( work, sx+sl, sy, sx, sy+sl, 45 );
                line( work, sx, sy+sl, sx-sl, sy, 45 );
                circlefill( work, sx, sy, rnd( sl / 2 ), 56 );

        }

}


void draw_mine() {

        for ( int q=0;q<max_mine;q++ )
        if ( mine[q].alive ) {

                circlefill( work, mine[q].x, mine[q].y, mine_rad, 41 );
                line( work, mine[q].x-2, mine[q].y-2, mine[q].x-6, mine[q].y-6, 31 );
                line( work, mine[q].x-1, mine[q].y+2, mine[q].x-7, mine[q].y+8, 31 );
                line( work, mine[q].x+3, mine[q].y-1, mine[q].x+7, mine[q].y-6, 31 );
                line( work, mine[q].x+2, mine[q].y+3, mine[q].x+8, mine[q].y+8, 31 );

        }

}


void end_round() {

        s_crys.alive = false;
        clear( border );
        guy.level++;
        draw_border();
        init_crystal();
        init_bullets();
        init_player();
        init_mine();
        init_evil();
        init_exit();
        if ( guy.level > 0 )
                make_mine( max_mine - ( 20 / guy.level ) );
        // make_crystal( max_crystal - ( 8 / (guy.level+1) ) );
	make_crystal( 4 );

}


void check_win() {

        bool cy = false;
        for ( int q = 0; q < max_crystal; q++ )
                if ( crys[q].alive )
                        cy = true;
        if ( cy ) return;
        gate.open = true;
        if ( guy.x-super_rad >= gate.x && guy.x+super_rad <= gate.x+super_rad*gate_length && guy.y-super_rad <= super_rad && gate.open )
                end_round();

}


void move_expl() {

        for ( int q = 0;q<max_explode; q++ )
        	if ( expl_r[q].alive ) {

                	expl_r[q].fx += expl_r[q].dx;
	                expl_r[q].fy += expl_r[q].dy;
        	        expl_r[q].x = (int)expl_r[q].fx;
	                expl_r[q].y = (int)expl_r[q].fy;
        	        expl_r[q].life--;
                	if ( expl_r[q].life <= 0 )
	                        expl_r[q].alive = false;

        	}

}


void find_col( int & me ) {

        int tk = me;
        if ( me >= 0 )
                tk = 41+72;
        if ( me > 4 )
                tk = 41;
        if ( me > 9 )
                tk = 44;
        if ( me > 13 )
                tk = 67;
        if ( me >= 17 )
                tk = 31;
        me = tk;

}


void draw_expl() {

        for ( int q = 0; q < max_explode; q++ )
        if ( expl_r[q].alive ) {

                int col = (2-(int)expl_r[q].speed)*10+20 - (20-expl_r[q].life);
                col = (int)( (double)(expl_r[q].life*2) / expl_r[q].speed );
                find_col( col );
                circlefill( work, expl_r[q].x, expl_r[q].y, 1, col );

        }

}


bool hit_bullet( int x, int y, int min) {

        for ( int q = 0; q < max_bullet; q++ )
                if ( guy_bullet[q].alive )
                        if ( av_dist( guy_bullet[q].x, guy_bullet[q].y, x, y, min*3 ) )
                        if ( dist( guy_bullet[q].x, guy_bullet[q].y, x, y ) <= min ) {
                                if ( !power[laser] )
                                        guy_bullet[q].alive = false;
                return true;
        }
        return false;

}


void set_evil_0( int who ) {

        evil[who].speed = (rnd(400)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 7;

}


void set_evil_1( int who ) {

        evil[who].speed = (rnd(700)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 7;

}


void set_evil_2( int who ) {

        evil[who].speed = (rnd(600)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 5;

}


void set_evil_3( int who ) {

        evil[who].speed = (rnd(600)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 6;

}


void set_evil_4( int who ) {

        evil[who].speed = (rnd(600)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 6;

}


void set_evil_5( int who ) {

        evil[who].speed = (rnd(400)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 6;

}


void set_evil_6( int who ) {

        evil[who].speed = (rnd(400)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 6;

}


void set_evil_7( int who ) {

        evil[who].speed = (rnd(350)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 7;

}


void set_evil_8( int who ) {

        evil[who].speed = (rnd(600)) / 200;
        evil[who].dx = tcos[evil[who].ang] * evil[who].speed;
        evil[who].dy = tsine[evil[who].ang] * evil[who].speed;
        evil[who].evil_rad = 10;

}


int get_kind( int q ) {

        switch( q ) {
                case 0   :  return 0;
                case 1   :  return 0;
                case 2   :  return 1;
                case 3   :  return ( rnd(2) );
                case 4   :  return 2;
                case 5   :  return ( rnd(3) );
                case 6   :  return 3;
                case 7   :  return ( rnd(4) );
                case 8   :  return 4;
                case 9   :  return ( rnd(5) );
                case 10  :  return 5;
                case 11  :  return ( rnd(6) );
                case 12  :  return 6;
                case 13  :  return ( rnd(7) );
                case 14  :  return 7;
                case 15  :  return ( rnd(8) );
                case 16  :  return 8;
                case 17  :  return ( rnd(9) );
        }                                         //switch

        return ( rnd(max_kind) );

}


void make_evil() {

        int q = 0;
        while ( q < max_evil && evil[q].alive )
                q++;
        if ( q >= max_evil )
                return;
        evil[q].alive = true;
        int place = rnd(2);
        switch ( place ) {
                case 0   :  { evil[q].x = super_rad*2; evil[q].y = 240; break; }
                case 1   :  { evil[q].x = screen_x-super_rad*2; evil[q].y = 240; break; }
        }                                         //switch
        evil[q].fx = evil[q].x;
        evil[q].fy = evil[q].y;
        evil[q].ang = rnd(360);
        evil[q].kind = get_kind( guy.level );
        /*
        if ( guy.level % 2 == 1 )
           evil[q].kind = random() % (guy.level);
        else
           evil[q].kind = guy.level;
        */

        switch( evil[q].kind ) {
                case 0   :  { set_evil_0( q ); break; }
                case 1   :  { set_evil_1( q ); break; }
                case 2   :  { set_evil_2( q ); break; }
                case 3   :  { set_evil_3( q ); break; }
                case 4   :  { set_evil_4( q ); break; }
                case 5   :  { set_evil_5( q ); break; }
                case 6   :  { set_evil_6( q ); break; }
                case 7   :  { set_evil_7( q ); break; }
                case 8   :  { set_evil_8( q ); break; }
        }

}


void act_evil_0( int q ) {
        if ( rnd(30) == rnd(30) )
                evil[q].speed = (rnd(400) )/200;
}


void act_evil_1( int q ) {
        if ( rnd(50) == rnd(50) )
                evil[q].ang = rnd(360);

        if ( rnd(75) == rnd(75) )
                evil[q].speed = (rnd(900)) / 200;
}


void act_evil_2( int q ) {

        if ( rnd(90) == rnd(90) )
                evil[q].speed = (rnd(600))/200;
        if ( rnd(55) == rnd(55) ) {
                int ang = rnd(360);
                double speed = (double)(rnd(200))/(double)50;
                if ( speed < 1 )
                        speed = 1;
                make_evil_bullet( evil[q].x, evil[q].y, tcos[ang]*speed, tsine[ang]*speed );
        }

}


void act_evil_3( int q ) {

        if ( rnd(90) == rnd(90) )
                evil[q].speed = (rnd(600))/200;
        if ( rnd(70) == rnd(70) ) {
                int ang = gang( evil[q].x, evil[q].y, guy.x, guy.y );
                double speed = 3.2;
                make_evil_bullet( evil[q].x, evil[q].y, tcos[ang]*speed, tsine[ang]*speed );
        }

}


void act_evil_4( int q ) {

        if ( rnd(50) == rnd(50) )
                evil[q].speed = (rnd(600))/200;
        if ( rnd(75) == rnd(75) ) {

                evil[q].ang = gang( evil[q].x, evil[q].y, guy.x, guy.y );
                evil[q].speed = 3.2;

        }

}


void act_evil_5( int q ) {

        if ( rnd(90) == rnd(90) )
                evil[q].speed = (rnd(320))/200;
        if ( rnd(25) == rnd(25) ) {
                int ang = rnd(360);
                double speed = (double)(rnd(125))/(double)50;
                if ( speed < 0.1 )
                        speed = 0.1;
                make_evil_bullet( evil[q].x, evil[q].y, tcos[ang]*speed, tsine[ang]*speed );
        }

}


void act_evil_6( int q ) {

        if ( rnd(80) == rnd(80) )
                evil[q].speed = (rnd(400))/200;
        for ( int x = 0; x < max_bullet; x++ )
                if ( guy_bullet[x].alive )
                if ( dist( guy_bullet[x].x, guy_bullet[x].y, evil[q].x, evil[q].y ) <= evil[q].evil_rad ) {

                #define min_xl 4.8
                        int ang = gang( evil[q].x, evil[q].y, guy.x, guy.y );
                make_evil_bullet( evil[q].x, evil[q].y, tcos[ang]*min_xl, tsine[ang]*min_xl );
                x = max_bullet;

        }

}


void act_evil_7( int q ) {
        if ( rnd(65) == rnd(65) )
                evil[q].speed = (rnd(350))/200;
        for ( int x = 0; x < max_bullet; x++ )
                if ( guy_bullet[x].alive )
                if ( dist( guy_bullet[x].x, guy_bullet[x].y, evil[q].x, evil[q].y ) <= evil[q].evil_rad ) {

                #define min_zl 4.3
                        for ( int ang = 0; ang < 360; ang += 15 )
                                make_evil_bullet( evil[q].x, evil[q].y, tcos[ang]*min_zl, tsine[ang]*min_zl );
                x = max_bullet;

        }
}


void act_evil_8( int q ) {
        if ( rnd(65) == rnd(65) )
                evil[q].speed = (rnd(350))/200;
        if ( rnd(60) == rnd(60) )
                evil[q].ang = rnd(360);

        for ( int x = 0; x < max_bullet; x++ )
                if ( guy_bullet[x].alive )
                if ( dist( guy_bullet[x].x, guy_bullet[x].y, evil[q].x, evil[q].y ) <= evil[q].evil_rad ) {

                for ( int zt = 0; zt < 2; zt ++ ) {

                        int take = 0;
                        while ( take < max_evil && evil[take].alive )
                                take++;
                        if ( take >= max_evil )
                                break;
                        evil[take].alive = true;
                        evil[take].x = evil[q].x;
                        evil[take].y = evil[q].y;
                        evil[take].fx = evil[take].x;
                        evil[take].fy = evil[take].y;
                        evil[take].ang = rnd(360);
                        evil[take].kind = zoomer;
                        evil[take].speed = rnd(1000) / 200 + 1;
                        evil[take].evil_rad = 5;

                }                                 //for loop

                make_expl( evil[q].x, evil[q].y );
                evil[q].alive = false;
                guy.score += 300;
                if (!power[laser] )
                        guy_bullet[x].alive = false;
                x = max_bullet;
        }
}


void act_evil_zoomer( int q ) {
        if ( rnd(50) == rnd(50) )
                evil[q].speed = rnd(1000) / 200 + 1;
        if ( rnd(45) == rnd(45) )
                evil[q].ang = rnd(360);
}


void move_evil() {
        for ( int q = 0; q < max_evil; q++ )
        if ( evil[q].alive ) {

                evil[q].fx += evil[q].dx;
                evil[q].fy += evil[q].dy;
                evil[q].x = (int)evil[q].fx;
                evil[q].y = (int)evil[q].fy;
                if ( evil[q].x > screen_x-super_rad*2 ) {
                        evil[q].x = screen_x-super_rad*2;
                        evil[q].fx = screen_x-super_rad*2;
                        evil[q].ang = (evil[q].ang+90)%360;
                }
                if ( evil[q].x < super_rad*2 ) {
                        evil[q].x = super_rad*2;
                        evil[q].fx = super_rad*2;
                        evil[q].ang = (evil[q].ang+90)%360;
                }
                if ( evil[q].y < super_rad*2 ) {
                        evil[q].y = super_rad*2;
                        evil[q].fy = super_rad*2;
                        evil[q].ang = (evil[q].ang+90)%360;
                }
                if ( evil[q].y > screen_y-super_rad*2 ) {
                        evil[q].y = screen_y-super_rad*2;
                        evil[q].fy = screen_y-super_rad*2;
                        evil[q].ang = (evil[q].ang+90)%360;
                }
                evil[q].dx = tcos[evil[q].ang] * evil[q].speed;
                evil[q].dy = tsine[evil[q].ang] * evil[q].speed;
                switch( evil[q].kind ) {
                        case 0   :  { act_evil_0( q ); break; }
                        case 1   :  { act_evil_1( q ); break; }
                        case 2   :  { act_evil_2( q ); break; }
                        case 3   :  { act_evil_3( q ); break; }
                        case 4   :  { act_evil_4( q ); break; }
                        case 5   :  { act_evil_5( q ); break; }
                        case 6   :  { act_evil_6( q ); break; }
                        case 7   :  { act_evil_7( q ); break; }
                        case 8   :  { act_evil_8( q ); break; }
                        case zoomer :  { act_evil_zoomer( q ); break; }
                }

                if ( evil[q].speed < 1 )
                        evil[q].speed = 1;

                if ( dist( guy.x, guy.y, evil[q].x, evil[q].y ) <= super_rad+evil[q].evil_rad ) {
                        make_expl( evil[q].x, evil[q].y );
                        make_expl( guy.x, guy.y );
                        evil[q].alive = false;
			died_player();
                        init_player();
                }

                if ( hit_bullet( evil[q].x, evil[q].y, evil[q].evil_rad ) ) {

                        make_expl( evil[q].x, evil[q].y );
                        evil[q].alive = false;
                        guy.score += 300;

                }

        }                                         //for loop
}


void draw_evil_0( int q ) {
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 35 );
}


void draw_evil_1( int q ) {
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 51 );
}


void draw_evil_2( int q ) {
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 71 );
}


void draw_evil_3( int q ) {
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 63 );
}


void draw_evil_4( int q ) {
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 56 );
}


void draw_evil_5( int q ) {

        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 49+72 );
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad/2, 29 );

}


void draw_evil_6( int q ) {

        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 33+72 );
        line( work, evil[q].x+(int)(tcos[45]*evil[q].evil_rad), evil[q].y+(int)(tsine[45]*evil[q].evil_rad),
                evil[q].x-(int)(tcos[45]*evil[q].evil_rad), evil[q].y-(int)(tsine[45]*evil[q].evil_rad), 37 );
        line( work, evil[q].x+(int)(tcos[135]*evil[q].evil_rad), evil[q].y+(int)(tsine[135]*evil[q].evil_rad),
                evil[q].x-(int)(tcos[135]*evil[q].evil_rad), evil[q].y-(int)(tsine[135]*evil[q].evil_rad), 37 );

}


void draw_evil_7( int q ) {

        for ( int z = evil[q].evil_rad-1; z >= 0; z-- )
                circlefill( work, evil[q].x, evil[q].y, z, 35+z*2 );

}


void draw_evil_8( int q ) {

        #define dl 7.5
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 27 );
        for ( int z = 0; z < 360; z+=40 )
                circlefill( work, evil[q].x+(int)(tcos[(z+(evil[q].x % 2)*15)%360]*dl),
                        evil[q].y+(int)(tsine[(z+(evil[q].x % 2)*15)%360]*dl),
                        1, 31 );

}


void draw_evil_zoomer( int q ) {
        circlefill( work, evil[q].x, evil[q].y, evil[q].evil_rad, 37+rnd(5) );
}


void draw_evil() {

        for ( int q = 0; q < max_evil; q++ )
                if ( evil[q].alive )
                switch( evil[q].kind) {
                        case 0   :  { draw_evil_0( q ); break; }
                        case 1   :  { draw_evil_1( q ); break; }
                        case 2   :  { draw_evil_2( q ); break; }
                        case 3   :  { draw_evil_3( q ); break; }
                        case 4   :  { draw_evil_4( q ); break; }
                        case 5   :  { draw_evil_5( q ); break; }
                        case 6   :  { draw_evil_6( q ); break; }
                        case 7   :  { draw_evil_7( q ); break; }
                        case 8   :  { draw_evil_8( q ); break; }
                        case zoomer :  { draw_evil_zoomer( q ); break; }
        }

}


void move_exit() {

        gate.fx += gate.dx;
        gate.x = (int)gate.fx;
        if ( gate.x+super_rad*gate_length >= screen_x-3 ) {
                gate.dx = -gate.dx;
                gate.x = screen_x-super_rad*gate_length-3;
                gate.fx = gate.x;
        }
        if ( gate.x <= 3 ) {
                gate.dx = -gate.dx;
                gate.x = 3;
                gate.fx = gate.x;
        }

}


void make_power() {

        if ( s_crys.alive == false ) {

                int place = rnd(2);
                switch ( place ) {
                        case 0   :  { s_crys.x = super_rad*2; s_crys.y = 240; break; }
                        case 1   :  { s_crys.x = screen_x-super_rad*2; s_crys.y = 240; break; }
                }                                 //switch
                s_crys.fx = s_crys.x;
                s_crys.fy = s_crys.y;
                double speed = rnd(2000) / 200 + 3;
                s_crys.ang = rnd(360);
                s_crys.dx = tcos[s_crys.ang]*speed;
                s_crys.dy = tsine[s_crys.ang]*speed;
                s_crys.alive = true;
                s_crys.power = rnd(max_power);
        }

}


void draw_exit() {

        int x1 = gate.x;
        int x2 = gate.x+super_rad*gate_length;
        rectfill( work, x1, 0, x2, super_rad-2, 41 * ( gate.open^1 ) );

}


void draw_score() {
	char * score = int2str( guy.score );
        if ( guy.x < 450 )
                aprint( work, font, 520, 460, 31, "SCORE:%s", score );
        else
                aprint( work, font, 20, 460, 31, "SCORE:%s", score );

	delete[] score;

        if ( guy.x > 200 )
                aprint( work, font, 30, 20, 31, "LIVES:%d", guy.lives );
        else
                aprint( work, font, 520, 20, 31, "LIVES:%d", guy.lives );

        if ( guy.y > 200 )
                aprint( work, font, 30, 30, 31, "BOMBS:%d", guy.bombs );
        else
                aprint( work, font, 30, 430, 31, "BOMBS:%d", guy.bombs );

}


void move_power() {

        if ( s_crys.alive == false )
                return;

        s_crys.fx += s_crys.dx;
        s_crys.fy += s_crys.dy;
        s_crys.x = (int)s_crys.fx;
        s_crys.y = (int)s_crys.fy;
        if ( s_crys.x < super_rad ) {
                s_crys.x = super_rad;
                s_crys.fx = s_crys.x;
                s_crys.dx = -s_crys.dx;
        }
        if ( s_crys.x > screen_x-super_rad-1 ) {
                s_crys.x = screen_x-super_rad-1;
                s_crys.fx = s_crys.x;
                s_crys.dx = -s_crys.dx;
        }
        if ( s_crys.y < super_rad ) {
                s_crys.y = super_rad;
                s_crys.fy = s_crys.y;
                s_crys.dy = -s_crys.dy;
        }
        if ( s_crys.y > screen_y-super_rad-1 ) {
                s_crys.y = screen_y-super_rad-1;
                s_crys.fy = s_crys.y;
                s_crys.dy = -s_crys.dy;
        }
        if ( dist( guy.x, guy.y, s_crys.x, s_crys.y ) < super_rad+power_rad ) {

                power[ s_crys.power ] = true;
                power_time[ s_crys.power ] = 10000;
                if ( power[ no_mine ] ) {

                        for ( int x = 0; x < max_mine; x++ )
                        if ( mine[x].alive ) {

                                make_expl( mine[x].x, mine[x].y );
                                mine[x].alive = false;

                        }

                }
                s_crys.alive = false;

        }
}


void move_all() {
        move_guy();
        move_bullet();
        move_evil_bullet();
        move_expl();
        move_evil();
        move_exit();
        move_power();
}


void draw_power() {

        if ( !s_crys.alive )
                return;
        circle( work, s_crys.x, s_crys.y, power_rad, 31 );

}


void draw_all() {

        blit( border, work, 0, 0, 0, 0, screen_x, screen_y );
        draw_score();
        draw_guy();
        draw_bullet();
        draw_evil_bullet();
        draw_crystal();
        draw_mine();
        draw_expl();
        draw_evil();
        draw_exit();
        draw_power();

}


int main() {

        first_init();
        end_round();
        while ( !key[KEY_ESC] && guy.lives > 0 ) {

		bool draw = false;
		while ( speed_counter > 0 ){
			draw = true;
	                if ( rnd(75) == rnd(75) )
        	                make_evil();
                //if ( random() % 100 == random() % 100 )
                //   make_power();
                /*
                if ( random() % ( 200 / ( guy.level+1 ) ) == random() % ( 200 / ( guy.level+1) ) )
                   make_evil();
                */
                	move_all();
			speed_counter--;
		}

		if ( draw ){
	                draw_all();
        	        blit( work, screen, 0, 0, 0, 0, screen_x, screen_y );
                	clear( work );
		}

        }

        destroy_bitmap( work );
        destroy_bitmap( border );

	return 0;

}
END_OF_MAIN()
