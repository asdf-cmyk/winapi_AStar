#include "stdafx.h"
#include <windows.h>
#include <algorithm>
#include <vector>

using namespace std;

#define MAPWIDTH 6
#define MAPHEIGHT 5

struct node {
	int f, g, h, x, y;
};

int verOrHor = 10;
int diagonal = 14;
node nodeMap[MAPHEIGHT][MAPWIDTH];
vector<POINT> pathCont;

void Init();
void AStar(POINT, POINT);

int main()
{
	Init();
	AStar({ 0, 0 }, { 5, 4 });

	return 0;
}

void Init()
{
	for (int y = 0; y < MAPHEIGHT; y++)
	{
		for (int x = 0; x < MAPWIDTH; x++)
		{
			nodeMap[y][x].f = 0;
			nodeMap[y][x].g = nodeMap[y][x].h = -1;
			nodeMap[y][x].x = x;
			nodeMap[y][x].y = y;
		}
	}

	nodeMap[3][3].f = -1;
}

void AStar(POINT srt, POINT dst)
{
	POINT curPt = srt;
	POINT tmpPt;

	pathCont.push_back(srt);
	while (curPt.x != dst.x || curPt.y != dst.y)
	{
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (!i && !j)
					continue;

				int Y = curPt.y + i;
				int X = curPt.x + j;
				if (Y < 0 || X < 0 ||
					Y >= MAPHEIGHT || X >= MAPWIDTH) continue;
				if (nodeMap[Y][X].f == -1)
					continue;
				if (nodeMap[Y][X].g != -1)
					continue;

				if (nodeMap[Y][X].g == -1)
					nodeMap[Y][X].g = 1 + diagonal;
				if (nodeMap[Y][X].g == -1)
					nodeMap[Y][X].g = 1 + verOrHor;

				int xDist = dst.x - X;
				int yDist = dst.y - Y;
				int hSum = 0;
				while (xDist && yDist)
				{
					hSum += diagonal;
					if (xDist > 0) xDist--;
					else if (xDist < 0) xDist++;
					if (yDist > 0) yDist--;
					else if (yDist > 0) yDist++;
				}

				int	distXorY = 0;
				if (!xDist && yDist)
					distXorY = abs(yDist);
				else if (xDist && !yDist)
					distXorY = abs(xDist);

				for (int i = 0; i < distXorY; i++)
					hSum += 10;

				nodeMap[Y][X].h = hSum;
				nodeMap[Y][X].f = nodeMap[Y][X].g +	nodeMap[Y][X].h;
				nodeMap[Y][X].f = nodeMap[Y][X].f;
			} // for j end
		} // for i end

		node minNode = { 999, 999, 999, 0, 0 };
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				if (!i && !j) continue;
				int Y = curPt.y + i;
				int X = curPt.x + j;
				if (Y < 0 || X < 0 ||
					Y >= MAPHEIGHT || X >= MAPWIDTH) continue;
				if (nodeMap[Y][X].f == -1)
					continue;

				if (minNode.f > nodeMap[Y][X].f)
					minNode = nodeMap[Y][X];
			}
		}
		tmpPt.x = curPt.x = minNode.x;
		tmpPt.y = curPt.y = minNode.y;
		pathCont.push_back(curPt);
	}

	for (unsigned int i = 0; i < pathCont.size(); i++)
		cout << pathCont[i].x << ", " << pathCont[i].y << endl;
}