#include <iostream>
#include <iomanip>
#include <sstream>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfFpt.h>
#include <rspf/base/rspfDFeature.h>
#pragma warning(push)
#pragma warning(disable : 4482)



//////////////////////////////////////////////////////////////////////////
////////////////////////////rspfDLine////////////////////////////////

rspfDLine::rspfDLine(rspfDpt Dpt1, rspfDpt Dpt2)
{
	setPoints(Dpt1, Dpt2);
}

void rspfDLine::setPoints(rspfDpt Dpt1, rspfDpt Dpt2)
{
	first = Dpt1;
	second = Dpt2;

	rho = 0;
	//角度为对应直线与X轴的夹角
	theta = 0;
	if( first.x == second.x)
	{
		// 如果与Y轴平行
		rho = (double)first.x;
		theta = PI / 2.0;
	} 
	else
	{
		theta = atan((second.y - first.y) / (double)(second.x - first.x));
		//float t0 = (line[1].x * line[0].y - line[0].x * line[1].y) * cos(theta);
		//float t1 = -line[0].x * sin(theta) + line[0].y * cos(theta);
		//float t2 = -line[1].x * sin(theta) + line[1].y * cos(theta);
		rho = -first.x * sin(theta) + first.y * cos(theta);
	}
}

double rspfDLine::DistanceFromPoint(rspfDpt pt) const
{
	rspfDpt disVector;
	rspfDpt intersection_Tmp;
	double len0_2 = sqrt(rspf::square<double>(first.x - second.x) + rspf::square<double>(first.y - second.y));
	double len0 = sqrt(len0_2);
	if(len0 < DBL_EPSILON)
	{
		// 如果两点相同
		return sqrt(rspf::square<double>(first.x - pt.x) + rspf::square<double>(first.y - pt.y));
	}
	//面积法求距离
	return fabs(CrossMultiplication(pt, first, second) / len0);
}

double rspfDLine::DistanceFromPoint2(const rspfDpt& p) const
{
	double dist = -p.x * sin(theta) + p.y * cos(theta) - rho;
	return dist;
	//double dist = -p.x * sin(theta) + p.y * cos(theta) - rho;
	//double dx = fabs(dist*cos(theta));
	//double dy = fabs(dist*sin(theta));
	//return rspfDpt(dx, dy);
}

rspfDpt rspfDLine::DistanceFromSegment(const rspfDLine& l) const
{
	double  d1 = DistanceFromPoint2(l.first);
	double d2 = DistanceFromPoint2(l.second);
	return rspfDpt(d1, d2);
	//rspfDpt d1 = DistanceFromPoint2(l.first);
	//rspfDpt d2 = DistanceFromPoint2(l.second);
	//return rspfDpt(sqrt(d1.x*d1.x+d1.y*d1.y), sqrt(d2.x*d2.x+d2.y*d2.y));
	//return rspfDpt(0.5*(d1.x+d2.x), 0.5*(d1.y+d2.y));
}

rspfDpt rspfDLine::DistanceAndSegment(const rspfDLine& l) const
{
	rspfDpt base_point1 = rspfDpt(0.0, 0.0);
	rspfDpt base_point2 = rspfDpt(6000.0, 6000.0);
	double dist1 =DistanceFromPoint2(base_point1) - l.DistanceFromPoint2(base_point1);
	double dist2 =DistanceFromPoint2(base_point2) - l.DistanceFromPoint2(base_point2);
	return rspfDpt(dist1, dist2);
	//return DistanceFromSegment(l);
	//rspfDpt dpt1 = DistanceFromSegment(l);
	//rspfDpt dpt2 = l.DistanceFromSegment(*this);
	//double d1 = sqrt(dpt1.x*dpt1.x + dpt1.y*dpt1.y);
	//double d2 = sqrt(dpt2.x*dpt2.x + dpt2.y*dpt2.y);
	//return rspfDpt(d1, d2);

	rspfDpt d1 = DistanceFromSegment(l);
	rspfDpt d2 = l.DistanceFromSegment(*this);
	//return rspfDpt(sqrt(d1.x*d1.x+d1.y*d1.y), sqrt(d2.x*d2.x+d2.y*d2.y));
	return rspfDpt(0.5*(fabs(d1.x)+fabs(d2.x)), 0.5*(fabs(d1.y)+fabs(d2.y)));
}

void rspfDLine::toPoint(rspfDpt *pt) const
{
	double delta_x = getFirstPoint().x - getSecondPoint().x;
	double delta_y = getFirstPoint().y - getSecondPoint().y;
	// x1*y2 - x2*y1
	double x1y2_x2y1 = getFirstPoint().x * getSecondPoint().y - getSecondPoint().x * getFirstPoint().y;
	double delta_x2 = delta_x * delta_x;
	double delta_y2 = delta_y * delta_y;
	double denominator = delta_x2+delta_y2+DBL_EPSILON;
	*pt = rspfDpt(delta_y*x1y2_x2y1/denominator, -delta_x*x1y2_x2y1/denominator);
}

void rspfDLine::getPointDistanceDeriv(rspfDpt dpt, double* partial_x, double* partial_y, double hdelta/* =1e-6*/) const
{
	*partial_x = -sin(theta);
	*partial_y = cos(theta);
	return;

	double den = 0.5/hdelta;
	double dis1 = DistanceFromPoint(rspfDpt(dpt.x + hdelta, dpt.y));
	double dis2 = DistanceFromPoint(rspfDpt(dpt.x - hdelta, dpt.y));
	*partial_x = dis1 - dis2;
	dis1 = DistanceFromPoint(rspfDpt(dpt.x, dpt.y + hdelta));
	dis2 = DistanceFromPoint(rspfDpt(dpt.x, dpt.y - hdelta));
	*partial_y = dis1 - dis2;
	*partial_x = *partial_x * den;
	*partial_y = *partial_y * den;
}

void rspfDLine::getDistanceDeriv(rspfDpt* partial_x, rspfDpt* partial_y) const
{
	double delta_x = getFirstPoint().x - getSecondPoint().x;
	double delta_y = getFirstPoint().y - getSecondPoint().y;
	double delta_x2 = delta_x * delta_x;
	double delta_y2 = delta_y * delta_y;
	double delta_xy = delta_x * delta_y;
	double denominator = delta_x2+delta_y2+DBL_EPSILON;
	double ay2 = delta_y2 / denominator;
	double axy = delta_xy / denominator;
	double ax2 = delta_x2 / denominator;
	*partial_x = rspfDpt(ay2, axy);
	*partial_y = rspfDpt(axy, ax2);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
///////////////////////////////rspfDArea////////////////////////////////
//////////////////////////////////////////////////////////////////////////
rspfDArea::rspfDArea(const vector<rspfDpt>& Points)
{
	m_Points = Points;

	// 保证闭合
	if(m_Points.size() > 1)
	{
		if(m_Points[0] != m_Points.back())
		{
			m_Points.push_back(m_Points[0]);
		}
	}
}

rspfDpt rspfDArea::DistanceFromPoint(rspfDpt pt) const
{
	double dis = 1048576.0;
	rspfDpt disVector;
	int nLeftIntersection = 0;
	int nRightIntersection = 0;

	for(unsigned int i = 0;i < m_Points.size() - 1;i++)
	{
		rspfDpt intersection;
		rspfDpt tmpDisVector = Distance2Segment(pt, m_Points[i], m_Points[i+1]);
		double tmpDis_2 = tmpDisVector.x * tmpDisVector.x + tmpDisVector.y * tmpDisVector.y;
		//double tmpDis = Distance2Straightline(pt, m_Points[i], m_Points[i+1], &intersection);
		int intersection_state = X_Intersection(pt, m_Points[i], m_Points[i+1], &intersection);

		if(DBL_EPSILON > tmpDis_2)
		{
			// 点在边界上
			return 0.0;
		}
		else
		{
			if(dis > tmpDis_2)
			{
				dis = tmpDis_2;
				disVector = tmpDisVector;
			}
		}
		if(intersection_state != 0 && isPointOnSegment(intersection, m_Points[i], m_Points[i+1]))
		{
			// 非水平且交点在线段上
			if(intersection_state < 0) nLeftIntersection++;
			else nRightIntersection++;			
		}
	}
	if((nLeftIntersection % 2 == 1) && (nRightIntersection % 2 == 1))
	{
		return rspfDpt(0.0, 0.0);
	}
	else
	{
		return disVector;
	}
}

rspfDpt rspfDArea::DistanceFromAreaMid(const rspfDArea &area, int *index/* = NULL*/) const
{
	// area中各顶点到该面特征距离的中值
	unsigned int nSize = area.m_Points.size() - 1;
	vector<unsigned int> indexList;
	vector<double> dis2List;
	vector<rspfDpt> disVectorList;
	for(unsigned int i = 0;i < nSize;i++)
	{
		rspfDpt tmpDisVector = DistanceFromPoint(area.m_Points[i]);
		disVectorList.push_back(tmpDisVector);
		dis2List.push_back(tmpDisVector.x * tmpDisVector.x + tmpDisVector.y * tmpDisVector.y);
		indexList.push_back(i);
	}

	for(unsigned int i = 0;i < nSize - 1;i++)
	{
		for(unsigned int j = i;j < nSize;j++)
		{
			if(dis2List[i] < dis2List[j])
			{
				double tmpDis = dis2List[i];
				dis2List[i] = dis2List[j];
				dis2List[j] = tmpDis;

				unsigned int tmpIndex = indexList[i];
				indexList[i] = indexList[j];
				indexList[j] = tmpIndex;

				rspfDpt tmpDisVector = disVectorList[i];
				disVectorList[i] = disVectorList[j];
				disVectorList[j] = tmpDisVector;
			}
		}
	}

	unsigned int midPos = nSize / 2;
	if(index) *index = indexList[midPos];
	return disVectorList[midPos];
}

// 
int rspfDArea::X_Intersection(rspfDpt pt, rspfDpt a, rspfDpt b, rspfDpt* intersection/* = NULL*/)const
{
	rspfDpt dpt1 = a - b;
	rspfDpt dpt2 = pt - a;
	if(fabs(dpt1.y) < DBL_EPSILON)
	{
		//如果水平
		intersection->makeNan();
		return 0;
	}
	else
	{
		intersection->y = pt.y;
		intersection->x = dpt1.x * dpt2.y / dpt1.y + a.x;
		double dis = intersection->x - pt.x;
		if(dis > 0) return 1;	//右相交
		else if(dis < 0) return -1;	//左相交
		else return 0;	//不相交
	}
}

double rspfDArea::Distance2Straightline(rspfDpt pt, rspfDpt a, rspfDpt b, rspfDpt* intersection/* = NULL*/)const
{
	double dis = rspf::nan();
	rspfDpt dpt1 = a - b;
	rspfDpt dpt2 = pt - a;
	if(fabs(dpt1.x) < DBL_EPSILON && fabs(dpt1.y) < DBL_EPSILON)
	{
		// 如果两点相同
		intersection->makeNan();
		dis = sqrt((pt.x - a.x) * (pt.x - a.x) + (pt.y - a.y) * (pt.y - a.y));
		return dis;
	}
	// X方向
	if(fabs(dpt1.y) < DBL_EPSILON)
	{
		//如果水平
		intersection->makeNan();
	}
	else
	{
		intersection->y = pt.y;
		intersection->x = dpt1.x * dpt2.y / dpt1.y + a.x;
		dis = intersection->x - pt.x;
	}
	if(fabs(dpt1.x) >= DBL_EPSILON)
	{
		//如果不垂直
		if(rspf::isnan(dis))
		{
			//如果水平
			dis = dpt1.y * dpt2.x / dpt1.x - dpt2.y;
		}
		else
		{
			if(dis >= 0.0)
				dis = min(fabs(dis), fabs(dpt1.y * dpt2.x / dpt1.x - dpt2.y));
			else
				dis = -min(fabs(dis), fabs(dpt1.y * dpt2.x / dpt1.x - dpt2.y));
		}
	}
	return dis;
}

rspfDpt rspfDArea::getPointDistanceDeriv_X(rspfDpt dpt, double  hdelta/* =1e-11*/) const
{   
	double den = 0.5/hdelta;
	rspfDpt res;
	res.x = DistanceFromPoint(rspfDpt(dpt.x + hdelta, dpt.y)).x;
	res.x -= DistanceFromPoint(rspfDpt(dpt.x - hdelta, dpt.y)).x;
	res.y = DistanceFromPoint(rspfDpt(dpt.x, dpt.y + hdelta)).x;
	res.y -= DistanceFromPoint(rspfDpt(dpt.x, dpt.y - hdelta)).x;
	res = res * den;
	return res;
}

rspfDpt rspfDArea::getPointDistanceDeriv_Y(rspfDpt dpt, double  hdelta/* =1e-11*/) const
{   
	double den = 0.5/hdelta;
	rspfDpt res;
	res.x = DistanceFromPoint(rspfDpt(dpt.x + hdelta, dpt.y)).y;
	res.x -= DistanceFromPoint(rspfDpt(dpt.x - hdelta, dpt.y)).y;
	res.y = DistanceFromPoint(rspfDpt(dpt.x, dpt.y + hdelta)).y;
	res.y -= DistanceFromPoint(rspfDpt(dpt.x, dpt.y - hdelta)).y;
	res = res * den;
	return res;
}

void rspfDArea::getPointDistanceDeriv(rspfDpt dpt, rspfDpt* partial_x, rspfDpt* partial_y, double hdelta/* =1e-6*/) const
{
	double den = 0.5/hdelta;
	rspfDpt disDpt1 = DistanceFromPoint(rspfDpt(dpt.x + hdelta, dpt.y));
	rspfDpt disDpt2 = DistanceFromPoint(rspfDpt(dpt.x - hdelta, dpt.y));
	partial_x->x = disDpt1.x - disDpt2.x;
	partial_y->x = disDpt1.y - disDpt2.y;
	disDpt1 = DistanceFromPoint(rspfDpt(dpt.x, dpt.y + hdelta));
	disDpt2 = DistanceFromPoint(rspfDpt(dpt.x, dpt.y - hdelta));
	partial_x->y = disDpt1.x - disDpt2.x;
	partial_y->y = disDpt1.y - disDpt2.y;
	*partial_x = *partial_x * den;
	*partial_y = *partial_y * den;
}
double CrossMultiplication(rspfDpt pt, rspfDpt a, rspfDpt b)
{
	return (a.x-pt.x)*(b.y-pt.y)-(b.x-pt.x)*(a.y-pt.y);
}

rspfDpt Distance2Segment(rspfDpt pt, rspfDpt a, rspfDpt b, rspfDpt* intersection/* = NULL*/) 
{
	rspfDpt disVector;
	rspfDpt intersection_Tmp;
	double len0_2 = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	double len0 = sqrt(len0_2);
	if(len0 < DBL_EPSILON)
	{
		// 如果两点相同
		if(intersection) *intersection = a;
		disVector.x = a.x - pt.x;
		disVector.y = a.y - pt.y;
		return disVector;
	}
	double d1_2 = (a.x - pt.x) * (a.x - pt.x) + (a.y - pt.y) * (a.y - pt.y);
	double d1 = sqrt(d1_2);
	double d2_2 = (b.x - pt.x) * (b.x - pt.x) + (b.y - pt.y) * (b.y - pt.y);
	double d2 = sqrt(d2_2);
	//面积法求距离
	double d0 = fabs(CrossMultiplication(pt, a, b) / len0);
	double d0_2 = d0 * d0;
	double len1_2 = d1_2 - d0_2;
	double len1 = sqrt(len1_2);
	double len2_2 = d2_2 - d0_2;
	double len2 = sqrt(len2_2);

	if (len1_2 > len0_2 || len2_2 > len0_2)
	{
		//垂足在线段外,取端点
		if (len1_2 > len2_2)
		{
			if(intersection) *intersection = b;
			disVector.x = b.x - pt.x;
			disVector.y = b.y - pt.y;
			return disVector;
		}
		else
		{
			if(intersection) *intersection = a;
			disVector.x = a.x - pt.x;
			disVector.y = a.y - pt.y;
			return disVector;
		}
	}
	//垂足在线段上
	intersection_Tmp.x = a.x + (b.x - a.x) * len1 / len0;
	intersection_Tmp.y = a.y + (b.y - a.y) * len1 / len0;
	if(intersection)
	{
		*intersection = intersection_Tmp;
	}
	disVector.x = intersection_Tmp.x - pt.x;
	disVector.y = intersection_Tmp.y - pt.y;
	return disVector;
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/////////////////////////////rspfDFeature///////////////////////////////
//////////////////////////////////////////////////////////////////////////
rspfDFeature::rspfDFeature()
{
	m_featureType = rspfDUnknown;
}
rspfDFeature::rspfDFeature(const rspfDFeature::rspfDFeatureType& type)
{
	m_featureType = type;
}
rspfDFeature::rspfDFeature(const rspfDPoint& Dpt)
{
	m_featureType = rspfDFeatureType::rspfDPointType;
	m_Points.clear();
	strId = Dpt.strId;
	m_Points.push_back(Dpt.point);
}

rspfDFeature::rspfDFeature(const rspfDLine& Line)
{
	m_featureType = rspfDFeatureType::rspfDLineType;
	m_Points.clear();
	m_Points.push_back(Line.first);
	m_Points.push_back(Line.second);
}

rspfDFeature::rspfDFeature(const rspfDArea& Area)
{
	m_featureType = rspfDFeatureType::rspfDAreaType;
	m_Points = Area.m_Points;
}

rspfDFeature::rspfDFeature(const rspfDFeature& src) :
strId(src.strId),
m_Points(src.m_Points),
m_featureType(src.m_featureType)
{
}
const rspfDFeature& rspfDFeature::operator = (const rspfDFeature &aFeature)
{
	strId = aFeature.strId;
	m_featureType = aFeature.m_featureType;
	m_Points = aFeature.m_Points;
	return *this;
}
//////////////////////////////////////////////////////////////////////////


rspfDpt rspfDArea::DistanceFromArea(const rspfDArea &area, int *index/* = NULL*/) const
{
	// area中各顶点到该面特征距离的最大值
	double dis = 0.0;
	rspfDpt disVector;
	int indexTmp = 0;
	for(unsigned int i = 0;i < area.m_Points.size();i++)
	{
		rspfDpt tmpDisVector = DistanceFromPoint(area.m_Points[i]);
		double tmpDis_2 = tmpDisVector.x * tmpDisVector.x + tmpDisVector.y * tmpDisVector.y;
		if(dis < tmpDis_2)
		{
			dis = tmpDis_2;
			disVector = tmpDisVector;
			indexTmp = i;
		}
	}
	if(index) *index = indexTmp;
	return disVector;
}


bool isPointOnSegment(rspfDpt pt, rspfDpt a, rspfDpt b)	
{
	// 线段左闭右开
	if(pt.isNan())
	{
		// 点无效
		return false;
	}
	rspfDpt dpt1 = a - b;
	rspfDpt dpt2 = pt - a;
	if(fabs(dpt1.x * dpt2.y - dpt1.y * dpt2.x) > DBL_EPSILON)
	{
		// 不在直线上
		return false;
	}
	else
	{
		return (pt.x >= min(a.x, b.x) && pt.x < max(a.x, b.x) && pt.y >= min(a.y, b.y) && pt.y <= max(a.y, b.y));
	}
}

#pragma warning(pop)