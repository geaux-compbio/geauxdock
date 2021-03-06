#include <mpi.h>
#include <cstdio>
#include <cstdlib>

#include <geauxdock.h>

MPI_Datatype MPI_JOB;


void
Dock (Complex *recv_msg, char * name, const int id)
{
  printf ("%-20s \t\t\t\t\t client %02d: %d %s\n",
          name, id, recv_msg->signal, recv_msg->lig_file);
}




void Client (int argc, char **argv, const int id)
{
  Complex *recv_msg = new Complex[1];


  int send_msg = id;
  const int dst = 0;
  MPI_Status status;
  const int mytag = 0;

  while (1) {
    MPI_Send (&send_msg, 1, MPI_INT, dst, mytag, MPI_COMM_WORLD);
    printf ("%-20s \t\t\t\t\t client %02d request\n", argv[0], id);
    MPI_Recv (recv_msg, 1, MPI_JOB, dst, mytag, MPI_COMM_WORLD, &status);

    if (recv_msg->signal == FINISH_SIGNAL) {
      printf ("%-20s \t\t\t\t\t\t\t\t\t\t client %02d retired\n", argv[0], id);
      break;
    }
    else {
      printf ("%s start docking\n", argv[0]);
      Dock (recv_msg, argv[0], id);
    }
  }



  delete[]recv_msg;
}



int
main (int argc, char **argv)
{
  int id, nprocs;
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &id);
  MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
  if (nprocs < 2) {
    printf ("nprocs < 2. exit\n");
    exit (-1);
  }

  MPI_Type_contiguous (sizeof (Complex), MPI_BYTE, &MPI_JOB);
  MPI_Type_commit (&MPI_JOB);


  if (id != 0)
    Client (argc, argv, id);
  else
    printf ("client is assigned with a wrong MPI rank\n");


  return 0;
}

