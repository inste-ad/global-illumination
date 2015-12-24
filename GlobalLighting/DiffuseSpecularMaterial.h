#pragma once

#include "Luminance.h"
#include "IMaterial.h"

#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace Engine;

namespace Materials
{
	class DuffuseSpecularMaterial: public IMaterial
	{
	public:
		DuffuseSpecularMaterial(GO_FLOAT rd[], GO_FLOAT rs[], int n[]) :
			rd(rd),
			rs(rs)
		{
			this->n[L_R] = n[L_R];
			this->n[L_G] = n[L_G];
			this->n[L_B] = n[L_B];
		}

		const Luminance BRDF(const Vector& direction, const Vector& ndirection, const Vector& normal) const
		{

			Luminance result = rd / M_PI; 
			
			const Vector R = 2 * normal.DotProduct(ndirection) * normal - ndirection;
			const GO_FLOAT cosphi = -direction.DotProduct(R);
			
			if(cosphi > 0 )
			{
				result += Luminance(
					rs.colors[L_R] == 0 ? 0 : rs.colors[L_R] * (n[L_R] + 2) * pow(cosphi, n[L_R]),
					rs.colors[L_G] == 0 ? 0 : rs.colors[L_G] * (n[L_G] + 2) * pow(cosphi, n[L_G]),
					rs.colors[L_B] == 0 ? 0 : rs.colors[L_B] * (n[L_B] + 2) * pow(cosphi, n[L_B])
					) / (2 * M_PI);
			}

			return result;
		}

		const RandomDirection SampleDirection(const Vector& direction, const Vector& point, const Vector& normal, const IShape& scene, GO_FLOAT absorption) const
		{	
			GO_FLOAT qd = (rd.colors[L_R] + rd.colors[L_G] + rd.colors[L_B]) / 3;
			GO_FLOAT qs = (rs.colors[L_R] + rs.colors[L_G] + rs.colors[L_B]) / 3;

			if(qd + qs + absorption > 1)
			{
				GO_FLOAT k = (1 - absorption) / (qd + qs);
				qd *= k;
				qs *= k;
			}
			
			GO_FLOAT ksi = (GO_FLOAT) rand() / RAND_MAX;

			if(ksi < qd)
			{
				GO_FLOAT cosa = (GO_FLOAT) rand() / RAND_MAX;
				GO_FLOAT sina = sqrt(1 - cosa * cosa);
				GO_FLOAT b = 2 * M_PI * (GO_FLOAT) rand() / RAND_MAX;

				Vector ndirection = Transform(normal, Vector(sina * cos(b), sina * sin(b), cosa));

				const HitPoint* nhp = scene.Intersection(point, ndirection);

				if(!nhp)
				{
					return RandomDirection();
				}

				return RandomDirection(rd / (2 * qd), nhp, ndirection);
			}
			else if(ksi - qd < qs)
			{
				GO_FLOAT selectedn = min(n[L_R], min(n[L_G], n[L_B]));
		
				GO_FLOAT cosa = pow((GO_FLOAT) rand() / RAND_MAX, 1 / (selectedn + 1));
				GO_FLOAT sina = sqrt(1 - cosa * cosa);
				GO_FLOAT b = 2 * M_PI * (GO_FLOAT) rand() / RAND_MAX;

				const Vector R = direction - 2 * normal.DotProduct(direction) * normal;
				const Vector ndirection = Transform(R, Vector(sina * cos(b), sina * sin(b), cosa));

				if(normal.DotProduct(ndirection) <= 0)
				{
					return RandomDirection();
				}

				const HitPoint* nhp = scene.Intersection(point, ndirection);

				if(!nhp)
				{
					return RandomDirection();
				}

				return RandomDirection(Luminance(
					rs.colors[L_R] == 0 ? 0 : rs.colors[L_R] * (n[L_R] + 2) * pow(cosa, n[L_R] - selectedn),
					rs.colors[L_G] == 0 ? 0 : rs.colors[L_G] * (n[L_G] + 2) * pow(cosa, n[L_G] - selectedn),
					rs.colors[L_B] == 0 ? 0 : rs.colors[L_B] * (n[L_B] + 2) * pow(cosa, n[L_B] - selectedn)
					) * normal.DotProduct(ndirection) / (qs * (selectedn + 2)), nhp, ndirection);
			}
			else
			{
				return RandomDirection();
			}
		}

	private:
		const Luminance rd; //koefficient diffuse reflection
		const Luminance rs; //koefficient specular reflection
		int n[3];
		
		Vector Transform(const Vector& axis, const Vector& direction) const
		{
			Vector t = axis;
			Vector M1;
			Vector M2;

			if(abs(axis.x) < abs(axis.y))
			{
				if(abs(axis.x) < abs(axis.z))
					t.x = 1;
				else
					t.z = 1;
			}
			else
			{
				if(abs(axis.y) < abs(axis.z))
					t.y = 1;
				else
					t.z = 1;
			}
	
			M1 = axis.CrossProduct(t).Normalize();
			M2 = axis.CrossProduct(M1);
	

			return Vector(
				direction.x * M1.x + direction.y * M2.x + direction.z * axis.x,
				direction.x * M1.y + direction.y * M2.y + direction.z * axis.y,
				direction.x * M1.z + direction.y * M2.z + direction.z * axis.z);
		}
	};
}