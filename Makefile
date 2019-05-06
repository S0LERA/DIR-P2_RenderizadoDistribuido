compilar:
	mpicc ./src/pract2.c -o ./src/pract2 -lX11

Renderizado:
	(cd src && mpirun -n 1 pract2)

limpiar:
	rm ./src/pract2
