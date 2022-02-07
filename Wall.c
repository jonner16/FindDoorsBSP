#include <stdio.h>
#include <math.h>
#include "Wall.h"

point* point_create(int x, int y){
	point* p = (point*)malloc(sizeof(point));
	p->x = (double)x;
	p->y = (double)y;
	return p;
}

segment* segment_create(point* start, point* end){
	segment* wall = (segment*)malloc(sizeof(segment));
	wall->start = start;
	wall->end = end;
	wall->orientation = GetOrientation(wall);
	return wall;
}

enum WallOrientation GetOrientation(segment* wall){
	if (wall->start->x == wall->end->x){
		return VERTICAL;
	} else {
		return HORIZONTAL;
	}
}

// determine a wall located on which side of the partition segment
enum ESegmentWallSide ClassifySegment(segment PartitionSeg, segment wall){
    int wOrient = GetOrientation(&wall);
    int pOrient = GetOrientation(&PartitionSeg);
    if(wOrient == VERTICAL && pOrient  == VERTICAL){
        if(wall.start->x > PartitionSeg.start->x){
            return SEGMENT_WALL_SIDE_BEHIND;
        }
        else if(wall.start->x <= PartitionSeg.start->x){
            return SEGMENT_WALL_SIDE_FRONT;
        }
    }
    if(wOrient == HORIZONTAL && pOrient == HORIZONTAL){
        if(wall.start->y > PartitionSeg.start->y){
            return SEGMENT_WALL_SIDE_BEHIND;
        }
        else if(wall.start->y <= PartitionSeg.start->y){
            return SEGMENT_WALL_SIDE_FRONT;
        }
    }
    if((wOrient == HORIZONTAL && pOrient == VERTICAL)){
        if(wall.start->x == PartitionSeg.start->x || wall.end->x == PartitionSeg.start->x || wall.end->x == PartitionSeg.end->x || wall.start->x == PartitionSeg.end->x){
            if(wall.start->x > PartitionSeg.start->x || wall.end->x > PartitionSeg.start->x){
                return SEGMENT_WALL_SIDE_BEHIND;
            }
            else if(wall.start->x < PartitionSeg.start->x || wall.end->x < PartitionSeg.start->x){
                return SEGMENT_WALL_SIDE_FRONT;
            }
        }
        else if((wall.start->x < PartitionSeg.start->x && wall.end->x > PartitionSeg.start->x) || (wall.end->x < PartitionSeg.start->x && wall.start->x > PartitionSeg.start->x)){
            return SEGMENT_WALL_SIDE_STRADDLING;
        }
        else if(wall.start->x < PartitionSeg.start->x){
            return SEGMENT_WALL_SIDE_FRONT;
        }
        else if(wall.start->x > PartitionSeg.start->x){
            return SEGMENT_WALL_SIDE_BEHIND;
        }
    }
    if(wOrient == VERTICAL && pOrient == HORIZONTAL){
        if(wall.start->y == PartitionSeg.start->y || wall.end->y == PartitionSeg.start->y){
            if(wall.start->y > PartitionSeg.start->y || wall.end->y > PartitionSeg.start->y){
                return SEGMENT_WALL_SIDE_BEHIND;
            }
            else if(wall.start->y < PartitionSeg.start->y || wall.end->y < PartitionSeg.start->y){
                return SEGMENT_WALL_SIDE_FRONT;
            }
        }
        else if((wall.start->y < PartitionSeg.start->y && wall.end->y > PartitionSeg.start->y) || (wall.end->y < PartitionSeg.start->y && wall.start->y > PartitionSeg.start->y)){
            return SEGMENT_WALL_SIDE_STRADDLING;
        }
        else if(wall.start->y < PartitionSeg.start->y){
            return SEGMENT_WALL_SIDE_FRONT;
        }
        else if(wall.start->y > PartitionSeg.start->y){
            return SEGMENT_WALL_SIDE_BEHIND;
        }
    }
    return -1;
}

// compute the intersection point on the wall divided by the PartitionSeg
int IntersectionWalls(segment PartitionSeg, segment wall, point* out){
    if(GetOrientation(&PartitionSeg) == HORIZONTAL){
                out->y = PartitionSeg.start->y;
                out->x = wall.start->x;
                return 1;
    }
    if(GetOrientation(&PartitionSeg) == VERTICAL){
                out->y = wall.start->y;
                out->x = PartitionSeg.start->x;
                return 1;
    }
    return -1;
}


// Output the two segments of the original wall divided by the partition segment to frontSeg and backSeg
void SplitWallSegment(segment PartitionSeg, segment wall, segment *frontSeg, segment *backSeg){
    if(ClassifySegment(PartitionSeg, wall) == SEGMENT_WALL_SIDE_STRADDLING){
        point* out = point_create(-1,-1);
        if(IntersectionWalls(PartitionSeg,wall,out) != -1){
            frontSeg->start = wall.start;
            frontSeg->end = out;
            backSeg->start = out;
            backSeg->end = wall.end;
        }
    }
}

