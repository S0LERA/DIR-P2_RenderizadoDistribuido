/* Pract2  RAP 09/10    Javier Ayllon*/

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <unistd.h>
#define NIL (0)

/*Variables Globales */

XColor colorX;
Colormap mapacolor;
char cadenaColor[] = "#000000";
Display *dpy;
Window w;
GC gc;

/*Funciones auxiliares */

void initX()
{

      dpy = XOpenDisplay(NIL);
      assert(dpy);

      int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
      int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

      w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
                              400, 400, 0, blackColor, blackColor);
      XSelectInput(dpy, w, StructureNotifyMask);
      XMapWindow(dpy, w);
      gc = XCreateGC(dpy, w, 0, NIL);
      XSetForeground(dpy, gc, whiteColor);
      for (;;)
      {
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == MapNotify)
                  break;
      }

      mapacolor = DefaultColormap(dpy, 0);
}

void dibujaPunto(int x, int y, int r, int g, int b)
{

      sprintf(cadenaColor, "#%.2X%.2X%.2X", r, g, b);
      XParseColor(dpy, mapacolor, cadenaColor, &colorX);
      XAllocColor(dpy, mapacolor, &colorX);
      XSetForeground(dpy, gc, colorX.pixel);
      XDrawPoint(dpy, w, gc, x, y);
      XFlush(dpy);
}

/* Programa principal */

int main(int argc, char *argv[])
{

      int rank, size;
      MPI_Comm commPadre;
      int tag;
      MPI_Status status;
      int hijos = 4;
      int buf[5];
      int error[hijos];

      MPI_Init(&argc, &argv);
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      MPI_Comm_size(MPI_COMM_WORLD, &size);
      MPI_Comm_get_parent(&commPadre);
      if ((commPadre == MPI_COMM_NULL) && (rank == 0))
      {
            initX();

            /* Codigo del maestro */
            MPI_Comm_spawn("pract2", MPI_ARGV_NULL, hijos, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &commPadre, error);

            for (int i = 0; i < 400*400; i++)
            {
                  MPI_Recv(&buf, 5, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, commPadre, MPI_STATUS_IGNORE);
                  dibujaPunto(buf[0], buf[1], buf[2], buf[3], buf[4]);
            }
            sleep(5);

            /*En algun momento dibujamos puntos en la ventana algo como
	dibujaPunto(x,y,r,g,b);  */
      }
      else
      {
            /* Codigo de todos los trabajadores */
            /* El archivo sobre el que debemos trabajar es foto.dat */
            MPI_File f;
            int lineas_hijo = 400 / hijos;
            int linea_inicio = lineas_hijo * rank;
            int linea_final = 0;
            if (rank == hijos - 1)
            {
                  linea_final = 400;
            }
            else
            {
                  linea_final = linea_inicio + lineas_hijo;
            }

            int bloque = lineas_hijo * 400 * 3 * sizeof(unsigned char);
            int bloque_hijo = bloque * rank;
            unsigned char tripleta[3];

            MPI_File_open(MPI_COMM_WORLD, "foto.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
            MPI_File_set_view(f, bloque_hijo, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, "native", MPI_INFO_NULL);

            for (int i = linea_inicio; i < linea_final; i++)
            {
                  for (int j = 0; j < 400; j++)
                  {
                        MPI_File_read(f, tripleta, 3, MPI_UNSIGNED_CHAR, &status);
                        buf[0] = j;
                        buf[1] = i;
                        buf[2] = (int)tripleta[0];
                        buf[3] = (int)tripleta[1];
                        buf[4] = (int)tripleta[2];
                        MPI_Send(&buf, 5, MPI_INT, 0, 1, commPadre);
                  }
            }
            MPI_File_close(&f);
      }

      MPI_Finalize();
      return 0;
}
