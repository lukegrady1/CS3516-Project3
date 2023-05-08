#include <stdio.h>
#include <stdlib.h>
#include "project3.h"
#include <string.h>

extern int TraceLevel;
extern float clocktime; //clocktime
extern int ConnectCosts[MAX_NODES][MAX_NODES]; //Connection cost

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};

struct distance_table dt0; //node's distance table of the whole system
struct NeighborCosts   *neighbor0; //used to print things
struct RoutePacket rpkt_0[MAX_NODES]; //DV sent to its neighbors

/* students to write the following two routines, and maybe some others */

void printdt0( int MyNodeNumber, struct NeighborCosts *neighbor,struct distance_table *dtptr );

int prev_DV_0[MAX_NODES]; //array that keeps a cached DV in respect to Node0

//get smallest distance and put it in the cached DV
void calc_min_cost0(){
    //calculate min cost
    int i;
    for(i=0;i<MAX_NODES;i++){
        prev_DV_0[i] = min_distance(dt0.costs[i]);
    }
}

//send packet from node 0
void send_pkt_0(){
  //make the packets
  int i;
   for(i=0;i<MAX_NODES;i++) {
       rpkt_0[i].sourceid = Node0_ID;
       rpkt_0[i].destid = i; //
       memcpy(rpkt_0[i].mincost, prev_DV_0, sizeof(prev_DV_0)); //copy of packets
   }

   // SEND TO ALL NEIGHBOURS

   for( i=0;i<MAX_NODES;i++) {
       if(i!=Node0_ID) {   //making sure it's not sending to itself
           toLayer2(rpkt_0[i]);
           printf("At time t=%.3f, node %d sends packet to node %d with: (%d  %d  %d  %d)\n",
                  clocktime, rpkt_0[i].sourceid, rpkt_0[i].destid, rpkt_0[i].mincost[0], rpkt_0[i].mincost[1],
                  rpkt_0[i].mincost[2],rpkt_0[i].mincost[3]);
       }
     }
}

//Check if DV has changed. if so send the new one to neighbors
void send_update_pkt0() {
  int old_min_cost[MAX_NODES];
  memcpy(old_min_cost, prev_DV_0, sizeof(prev_DV_0));
  int change = 0; //boolean to check if any data doesnt match the cache
  calc_min_cost0();
  int i;
  for(i=0;i<MAX_NODES;i++){
      if(old_min_cost[i]!=prev_DV_0[i]){
          change = 1;
      }
  }
  if(change == 1){ //the min cost changed have to send the new packets out
      send_pkt_0();
  }
  else
      printf("\n Min cost didn't change. \n");
}

//init DV based on the neighbors
void rtinit0() {
    printf("At time %0.3f, rtinit0() was called \n", clocktime);
    int i;
    int j;
    
    //initializes the table with the direct costs
    for( i=0;i<MAX_NODES;i++){
       for( j=0;j<MAX_NODES;j++){
         if(i==j){
           dt0.costs[i][j] =  ConnectCosts[Node0_ID][j]; //diagonals are direct neighbors

           //init the DV for itself
           if(i==Node0_ID){
             prev_DV_0[Node0_ID] = 0;//always 0 in reference to itself
           }else{
             prev_DV_0[i] = dt0.costs[i][j];// = diagonals originally
           }

         }
        else{
          dt0.costs[i][j] = INFINITY; 
          }
      }
    }

    neighbor0 = getNeighborCosts(Node0_ID);

    printdt0(Node0_ID,neighbor0,&dt0);

    send_pkt_0();
}

//update when vector received 
void rtupdate0( struct RoutePacket *rcvdpkt ) {
      int src = rcvdpkt->sourceid;
      int dest = rcvdpkt->destid;
      int mincost[MAX_NODES];  //clone of the received packets minimum cost
      int j;
      for(j= 0; j<MAX_NODES;j++){
          mincost[j] =  rcvdpkt->mincost[j];
        }

      //update the distance table
      int i;
      for(i=0;i<MAX_NODES;i++){
           //use the already calculated min_cost_0 path to all node
          int possibleValue = dt0.costs[src][src] + mincost[i];
          if(possibleValue<INFINITY){
                dt0.costs[i][src] = possibleValue;
          }
          else{
                dt0.costs[i][src] = INFINITY;
          }

      }


      neighbor0 = getNeighborCosts(Node0_ID);

      printdt0(Node0_ID,neighbor0,&dt0);

      send_update_pkt0();
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
void printdt0( int MyNodeNumber, struct NeighborCosts *neighbor,struct distance_table *dtptr ) {
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
}    // End of printdt0