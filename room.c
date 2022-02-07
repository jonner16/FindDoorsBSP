// room.c
#include <cairo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "Wall.h"
#include "List.h"
#include "BSP.h"


// Two line segments are equal if they start and end at the same point
// Returns 0 if segments are equal, -1 otherwise
int SegmentsEqual(void* segment1, void* segment2){
    segment* seg1 = (segment*) segment1;
    segment* seg2 = (segment*) segment2;
    if(seg1->start->x == seg2->start->x && seg1->start->y == seg2->start->y){
        if(seg1->end->x == seg2->end->x && seg1->end->y == seg2->end->y){
            return 0;
        }
    }
    return -1;
}

//print out the coordinates of the segment, remember to use %d for integers
void PrintSegment(void* segment1){
    segment* seg1 = (segment*) segment1;
    printf("%d %d %d %d\n", seg1->start->x,seg1->start->y,seg1->end->x,seg1->end->y);
}

// remember to free the start and end points of the segment
void DestroySegment(segment* seg){
    free(seg->start);
    free(seg->end);
    free(seg);
}

/**
* @brief Takes a parent node and generates a list containing all elements that form the left subtree of parent
* Note: The original list should be modified so that it contains only elements that form the right subtree of parent
* Assumes that parent is not in original
*
* @param parent The parent node which elements in the list argument will be compared with to determine left/right relationship
* @param original A pointer to a list containing all children of the parent. Original is modified to contain all right children
*
* @return ListPtr pointing to a list which holds all elements which will form the left subtree of parent. Returns NULL if parent or orignal
* are null
*/
//GetPartitions will check if the parent exists and if so creates a "left" list which signifies the //left node of the given "parent" param. Everything else stays in "remaining" param which stores the//right node of our subtree
ListPtr GetPartitions(void* parent, ListPtr remaining){
    if(parent != NULL){
        ListPtr left = list_create(SegmentsEqual, PrintSegment, (void*)DestroySegment);
        segment *partition = (segment*)parent;
        for(int i = 0; i < list_length(remaining); i++){
            segment* wall = (segment*)list_get_index(remaining,i);
            int classify = ClassifySegment(*partition, *wall);
            if(classify == SEGMENT_WALL_SIDE_FRONT){
                list_append(left, list_del_index(remaining,i));
                i--;
            }
            else if(classify == SEGMENT_WALL_SIDE_STRADDLING){
                point *point1 = point_create(-1,-1);
                point *point2 = point_create(-1,-1);
                segment *frontSeg = segment_create(point1,point2);
                segment *backSeg = segment_create(point1,point2);
                SplitWallSegment(*partition,*wall,frontSeg,backSeg);
                if(ClassifySegment(*partition,*frontSeg) != SEGMENT_WALL_SIDE_FRONT){
                	segment* temp = frontSeg;
                	frontSeg = backSeg;
                	backSeg = temp;
            	}
		list_del_index(remaining,i);
                list_append(left, frontSeg);
                list_insert(remaining, i, backSeg);
                free(point1);
                free(point2);
                i--;
            }
        }
        return left;
    }
    return NULL;
}
/**
 * @brief used to select the data which will form the root of a new subtree
 *
 * @param remaining A listpointer containing all the remaining data, after execution it should not contain the removed value
 *
 * @return void* A pointer to the extracted data
 */
//returns the new nodes data for our next leaf in the tree with one edge case handling if our list 
//remaining is only of length 1 
void* SelectPartitionRoot(ListPtr remaining){
    if(list_length(remaining) > 1)
        return list_del_index(remaining,(list_length(remaining)/2) - 1);
    else
        return list_del_index(remaining, 0);
}

/*
 *  * Used by Find
 *   */
int IntersectionDoors(segment PartitionSeg, segment wall){
    if(ClassifySegment(PartitionSeg, wall) == SEGMENT_WALL_SIDE_STRADDLING) {
        return 1;
    }
    PartitionSeg.orientation = GetOrientation(&wall);
    if (GetOrientation(&wall) == HORIZONTAL) {
        if(wall.start->y == PartitionSeg.start->y)
            return 1;
    }
    if (GetOrientation(&wall) == VERTICAL) {
        if(wall.start->x == PartitionSeg.start->x)
            return 1;
    }
    return -1;
}
/**
 * BONUS POINT CASE.
 * Determines which wall the Fancy Door is located on using bsp tree
 * The fancy door is a segment with given endpoints
 *
 * @param tree The BSP tree you built with the provided rooms
 * @param door A segment to be found
 * @param Wall It will store the wall contains the door here.
 *
 * @return int Returns -1 if the door outside the room (as long as part of the door is outside the room), 
 *						0 if the door inside the room (including the door touch the walls), 
 *						1 if the door on the walls
*/ 
int FindFancyDoor(BSPTree* tree, segment fancydoor, segment* Wall);

/**
 * Determines which wall the door is located on using bsp tree
 * The door is a point with given coordinates
 *
 * @param tree The BSP tree that you built with the provided rooms
 * @param door A coordinate point to be found
 * @param Wall It will store the wall contains the door here.
 *
 * @return int Returns -1 if the door outside the room, 0 if the door inside the room, 1 if the door on the walls
 */
int FindDoor(BSPTree* tree, point door, segment* Wall){
    int intersect = 0;
    ListPtr treeList = BSP_GetTraversal(tree);
    segment *doorSeg = segment_create(&door, &door);
    for(int i = 0; i < list_length(treeList); i++){
        segment *cur = list_get_index(treeList, i);
        doorSeg->orientation = GetOrientation(cur);
        if(GetOrientation(cur) == HORIZONTAL){
            if(cur->start->y == doorSeg->start->y) {
                if ((cur->start->x <= doorSeg->start->x && cur->end->x >= doorSeg->start->x) || (cur->start->x >= doorSeg->start->x && cur->end->x <= doorSeg->start->x)) {
                   Wall->start = cur->start;
                   Wall->end = cur->end;
                    return 1;
                }
            }
            doorSeg->orientation = VERTICAL;
        }
        else if(GetOrientation(cur) == VERTICAL){
            if(cur->start->x == doorSeg->start->x) {
                if ((cur->start->y <= doorSeg->start->y && cur->end->y >= doorSeg->start->y) || (cur->start->y >= doorSeg->start->y && cur->end->y <= doorSeg->start->y)) {
                    Wall->start = cur->start;
                    Wall->end = cur->end;
                    return 1;
                }
            }
            doorSeg->orientation = HORIZONTAL;
        }
        if(IntersectionDoors(*doorSeg, *cur) != -1){
            intersect++;
        }
    }
    if(intersect % 2 == 0){
        return -1;
    }else if(intersect % 2 == 1) {
        return 0;
    }
}


/**
 * @brief Used to draw out our BSP (room)
 *
 * @param tree A pointer to the tree containing the room
 * @param filename The name of the outputfile, DrawBSP concatenates with ".png"
 * @param doors A list of doors (points) that will be drawn to the cairo canvas in red
 * @param fancyDoors A list of fancyDoors (segments) that will be drawn to the cairo canvas in yellow
 *
 */
void DrawBSP(BSPTree* tree, char* filename, ListPtr doors, ListPtr fancyDoors){
	// Get a list of our BSP data sorted by pre-order traversal
	ListPtr traversalList = BSP_GetTraversal(tree);
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 512, 512);
	cairo_t *cr = cairo_create (surface);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_rectangle(cr, 0, 0, 512, 512);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_set_line_width(cr, 5.0);
	cairo_set_font_size(cr, 32);
	char label[10];
	for(int i = 0; i < list_length(traversalList); i++){
		segment* seg = (segment*)list_get_index(traversalList, i);
		//draw the line
		cairo_move_to(cr, seg->start->x, seg->start->y);
		cairo_line_to(cr, seg->end->x, seg->end->y);
		cairo_stroke(cr);
		int midX = (seg->start->x + seg->end->x)/2;
		int midY = (seg->start->y + seg->end->y)/2;
		cairo_move_to(cr, midX, midY);
		sprintf(label, "%d", i);
		cairo_show_text(cr, label);
	}
	
	if(doors != NULL){
		cairo_close_path(cr);
		cairo_set_source_rgb(cr, 1, 0, 0);
		for(int i = 0; i < list_length(doors); i++){
			point* d = (point*)list_get_index(doors, i);
			cairo_rectangle(cr, (d->x) - 5, (d->y) - 5, 10, 10);
			cairo_stroke(cr);
		}
	}

	if(fancyDoors != NULL){
		cairo_set_source_rgb(cr, 1, 1, 0);
		for(int i = 0; i < list_length(fancyDoors); i++){
			segment* seg = (segment*)list_get_index(fancyDoors, i);
			//draw the line
			cairo_move_to(cr, seg->start->x, seg->start->y);
			cairo_line_to(cr, seg->end->x, seg->end->y);
			cairo_close_path(cr);
			cairo_stroke(cr);
		}
	}

	cairo_destroy(cr);
	char* outputName = strcat(filename, ".png");
	cairo_surface_write_to_png(surface, outputName);
	cairo_surface_destroy(surface);
}


int main (int argc, char **argv){
    int SIZE = 100;
    char input[SIZE];
    char filename[SIZE];
    char doorType[SIZE];
    int x1; int y1; int x2; int y2;
    ListPtr roomList = list_create(SegmentsEqual, PrintSegment, (void*)DestroySegment);
    ListPtr doorList = list_create(SegmentsEqual, PrintSegment, (void*)DestroySegment);
    FILE *room = fopen(argv[1],"r");
    fgets(filename, SIZE, room);
    while(fgets(input, SIZE, room)){
        x1 = atoi(strtok(input, " "));
        y1 = atoi(strtok(NULL, " "));
        x2 = atoi(strtok(NULL, " "));
        y2 = atoi(strtok(NULL, " "));
        list_append(roomList,segment_create(point_create(x1,y1), point_create(x2,y2)));
    }
    FILE *doors = fopen(argv[2],"r");
    fgets(doorType, SIZE, doors);
    while(fgets(input, SIZE, doors)){
	x1 = atoi(strtok(input, " "));
	y1 = atoi(strtok(NULL, " "));
	list_append(doorList,point_create(x1,y1));
    }
    BSPTree *tree = BSP_Create(roomList, SelectPartitionRoot, GetPartitions, SegmentsEqual, PrintSegment, (void*)DestroySegment);
    for(int i = 0; i < list_length(doorList); i++){
	segment* wall = segment_create(point_create(-1,-1), point_create(-1,-1));
   	switch(FindDoor(tree,(*(point*)list_get_index(doorList,i)),wall)){
	case -1:
		printf("OUT\n");
		break;
	case 0: 
		printf("IN\n");
		break;
	case 1:
		PrintSegment(wall);
		break;
	default:
		printf("ERROR\n");
		break;
	}  
    }
    DrawBSP(tree, filename, doorList, NULL);
}
