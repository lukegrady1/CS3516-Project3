#include <stdio.h>
#include <string.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime; //clocktime
extern int ConnectCosts[MAX_NODES][MAX_NODES]; //Connection cost

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};

struct distance_table dt3; //node's distance table of the whole system
struct NeighborCosts   *neighbor3; //used to print things
struct RoutePacket rpkt_3[4]; //DV sent to its neightbors

/* students to write the following two routines, and maybe some others */
void printdt3( int MyNodeNumber, struct NeighborCosts *neighbor,struct distance_table *dtptr );
int prev_DV_3[MAX_NODES]; //array that keeps the a cached DV in respect to Node0

//get smallest distance and put it in the cached DV
void calc_min_cost3(){
    //calculate the min cost
    int i;
    for( i=0;i<MAX_NODES;i++){
        prev_DV_3[i] = min_distance(dt3.costs[i]);
    }
}

//send packet
void send_pkt_3(){
  //make the packets
  int i;
   for(i=0;i<MAX_NODES;i++) {
       rpkt_3[i].sourceid = Node3_ID;
       rpkt_3[i].destid = i; //
       memcpy(rpkt_3[i].mincost, prev_DV_3, sizeof(prev_DV_3)); //copy packets
   }

   // send to neighbors
   for(i=0;i<MAX_NODES;i++) {
       if(i!=Node3_ID) {   //not to itself
           toLayer2(rpkt_3[i]);
           printf("At time t=%.3f, node %d sends packet to node %d with: (%d  %d  %d  %d)\n",
                  clocktime, rpkt_3[i].sourceid, rpkt_3[i].destid, rpkt_3[i].mincost[0], rpkt_3[i].mincost[1],
                  rpkt_3[i].mincost[2],rpkt_3[i].mincost[3]);
       }
     }
}

//if dv changed send new one to neighbors
void send_update_pkt3() {
  int old_min_cost[MAX_NODES];
  memcpy(old_min_cost, prev_DV_3, sizeof(prev_DV_3));
  int change = 0;//boolean value to check if any data doesnt match cache
  calc_min_cost3();
  int i;
  for(i=0;i<MAX_NODES;i++){
      if(old_min_cost[i]!=prev_DV_3[i]){
          change = 1;
      }
  }
  if(change == 1){    //min cost changed sending new packets
      send_pkt_3();
  }
  else
      printf("\n Min cost didn't change. \n");
}


//init DV based on neightbors
void rtinit3() {
    printf("At time %0.3f, rtinit3() was called \n", clocktime);

    //initialize table with the direct costs
    int i,j;
    for( i=0;i<MAX_NODES;i++){
       for( j=0;j<MAX_NODES;j++){
         if(i==j){
           dt3.costs[i][j] =  ConnectCosts[Node3_ID][j]; //diagonals = direct neighbors

           //init the DV for itself
           if(i==Node3_ID){
             prev_DV_3[Node3_ID] = 0;//0 when referencing itself
           }else{
             prev_DV_3[i] = dt3.costs[i][j];// = diagonals initally
           }

         }
        else{
          dt3.costs[i][j] = INFINITY; 
          }
      }
    }

    neighbor3 = getNeighborCosts(Node3_ID);

    printdt3(Node3_ID,neighbor3,&dt3);

    send_pkt_3();
}

//update when vector received
void rtupdate3( struct RoutePacket *rcvdpkt ) {
      int src = rcvdpkt->sourceid;
      int dest = rcvdpkt->destid;
      int mincost[MAX_NODES];//clone received packets min cost
      int j;
      for( j= 0; j<MAX_NODES;j++){
          mincost[j] =  rcvdpkt->mincost[j];
        }

      //update table
      int i;
      for( i=0;i<MAX_NODES;i++){
          int possibleValue = dt3.costs[src][src] + mincost[i];
          if(possibleValue<INFINITY){
                dt3.costs[i][src] = possibleValue;
          }
          else{
                dt3.costs[i][src] = INFINITY;
          }

      }

      neighbor3 = getNeighborCosts(Node3_ID);

      printdt3(Node3_ID,neighbor3,&dt3);

      send_update_pkt3();
}
/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt3( int MyNodeNumber, struct NeighborCosts *neighbor,
		struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt3