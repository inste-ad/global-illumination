#include "stdafx.h"
#include "PhotonMap.h"

using namespace Engine;

class Engine::PhotonMapNode
{
public:
	PhotonMapNode(Photon photon, PhotonMapNode* left, PhotonMapNode* right, short axis) :
		photon(photon),
		left(left),
		right(right),
		axis(axis)
	{
	}

	~PhotonMapNode()
	{
		if(left != 0)
		{
			delete left;
		}
		
		if(right != 0)
		{
			delete right;
		}
	}
	Photon photon;
	PhotonMapNode* left;
	PhotonMapNode* right;
	short axis;
};


void PhotonMap::QSort(int left, int right, short axis)
{
    int l = left;
    int r = right;
    int m = left;

    while (l <= r)
    {
		while (photons[indexes[axis][l]].point[axis] < photons[indexes[axis][m]].point[axis] && l < right) l++;
        while (photons[indexes[axis][m]].point[axis] < photons[indexes[axis][r]].point[axis] && r > left) r--;

        if (l <= r)
        {
			int tmp = indexes[axis][l];
			indexes[axis][l] = indexes[axis][r];
			indexes[axis][r] = tmp;
            l++;
            r--;
        }
    }

    if (left < r) QSort(left, r, axis);
    if (l < right) QSort(l, right, axis);
}

PhotonMapNode* PhotonMap::CreateSubTree(int left, int right)
{
	int count = right - left + 1;

	if(count == 1)
	{
		return new PhotonMapNode(photons[indexes[0][left]], 0, 0, -1);
	}
	else
	{
		GO_FLOAT  distance[3];
		distance[0] = photons[indexes[0][right]].point.x - photons[indexes[0][left]].point.x;
		distance[1] = photons[indexes[1][right]].point.y - photons[indexes[1][left]].point.y;
		distance[2] = photons[indexes[2][right]].point.z - photons[indexes[2][left]].point.z;

		//axis = 0 for x, 1 for y, 2 for z
		short axis;
		short forSort_axis1;
		short	forSort_axis2;

		if(distance[0] > distance[1])
		{
			forSort_axis1 = 1;
			if(distance[0] > distance[2])
			{
				axis = 0;
				forSort_axis2 = 2;
			}
			else
			{
				axis = 2;
				forSort_axis2 = 0;
			}
		}
		else 
		{
			forSort_axis1 = 0;
			if(distance[1] > distance[2])
			{
				axis = 1;
				forSort_axis2 = 2;
			}
			else
			{
				axis = 2;
				forSort_axis2 = 1;
			}
		}

		for(int i = left; i <= right; i++)
		{
			indexes[forSort_axis1][i] = indexes[axis][i];
			indexes[forSort_axis2][i] = indexes[axis][i];
		}

		int m = left + (right - left) / 2;
		int mediana = indexes[axis][m];
		
		if(left == right - 1)
		{
			return new PhotonMapNode(photons[mediana], 0, CreateSubTree(m + 1, right), axis);
		}
		
		QSort(left, m - 1, forSort_axis1);
		QSort(left, m - 1, forSort_axis2);
		QSort(m + 1, right, forSort_axis1);
		QSort(m + 1, right, forSort_axis2);
	
		return new PhotonMapNode(photons[mediana], CreateSubTree(left, m - 1), CreateSubTree(m + 1, right), axis);
	}
}

PhotonMap::PhotonMap(int nphotons, Photon* photons) :
	photons(photons)
{
	indexes[0] = new int[nphotons];
	indexes[1] = new int[nphotons];
	indexes[2]= new int[nphotons];

	for(int i = 0; i < nphotons; i++)
	{
		indexes[0][i] = i;
		indexes[1][i] = i;
		indexes[2][i] = i;
	}

	int last_index = nphotons - 1;
	QSort(0, last_index, 0);
	QSort(0, last_index, 1);
	QSort(0, last_index, 2);
	root = CreateSubTree(0, last_index);
	
	delete[] indexes[0];
	delete[] indexes[1];
	delete[] indexes[2];
}

PhotonMap::~PhotonMap()
{
	if(root != 0)
		delete root;
}

void PhotonMap::GoDown(GO_FLOAT min[3], GO_FLOAT max[3], PhotonMapNode* node, const ParamsForFind& paramsForFind) const
{
	short axis = node->axis;
	
	if(paramsForFind.x[axis] <= node->photon.point[axis])
	{
		max[axis] = node->photon.point[axis];
		if(node->left)
		{
			GoDown(min, max, node->left, paramsForFind);
		}
		else
		{
		}
	}
	else
	{
		min[axis] = node->photon.point[axis];
		
		if(node->left)
		{
			GoDown(min, max, node->right, paramsForFind);
		}
		else
		{
		}
	}
}

void PhotonMap::FindNearest(const Vector& x, int count, Photon** result) const
{
	
}

