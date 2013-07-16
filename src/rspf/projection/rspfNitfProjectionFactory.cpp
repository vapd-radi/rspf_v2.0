//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Matt Revelle
//          David Burken
//
// Description:
//
// Contains class definition for rspfNitfProjectionFactory.
//
// $Id: rspfNitfProjectionFactory.cpp 20457 2012-01-13 20:08:49Z dburken $
//----------------------------------------------------------------------------

#include <rspf/projection/rspfNitfProjectionFactory.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfNitfTileSource.h>
#include <rspf/projection/rspfBilinearProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfMgrs.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/support_data/rspfNitfBlockaTag.h>
#include <rspf/support_data/rspfNitfFile.h>
#include <rspf/support_data/rspfNitfFileHeader.h>
#include <rspf/support_data/rspfNitfImageHeader.h>
#include <fstream>
#include <cmath>

// Define Trace flags for use within this file:
static rspfTrace traceDebug(rspfString("rspfNitfProjectionFactory:debug"));

rspfNitfProjectionFactory* rspfNitfProjectionFactory::theInstance = 0;

rspfNitfProjectionFactory::rspfNitfProjectionFactory()
{
}

rspfNitfProjectionFactory::~rspfNitfProjectionFactory()
{
}

rspfNitfProjectionFactory* rspfNitfProjectionFactory::instance()
{
	if(!theInstance)
	{
		theInstance = new rspfNitfProjectionFactory();
	}

	return theInstance;
}

rspfProjection*
rspfNitfProjectionFactory::createProjection(const rspfFilename& filename,
											rspf_uint32 entryIdx)const
{
	static const char MODULE[]="rspfNitfProjectionFactory::createProjection";
	if (traceDebug())
	{
		rspfNotify(rspfNotifyLevel_DEBUG)
			<< MODULE << " DEBUG:"
			<< "\nfilename:  " << filename
			<< "\nentryIdx:  " << entryIdx
			<< std::endl;
	}

	// See if there is an external geomtry.
	rspfProjection* result = createProjectionFromGeometryFile(filename, entryIdx);
	if (result)
	{
		return result;
	}

	if(!isNitf(filename))
	{
		return result; // result is NULL
	}

	rspfRefPtr<rspfNitfFile> nitf = new rspfNitfFile();
	if (!nitf->parseFile(filename))
	{
		return result; // result is NULL
	}

	long imageIndex = static_cast<long>(entryIdx);
	if ( imageIndex > (nitf->getHeader()->getNumberOfImages()-1) )
	{
		return result;
	}

	rspfRefPtr<rspfNitfImageHeader> imageHeader = nitf->getNewImageHeader(imageIndex);
	{
		if (!imageHeader)
		{
			return result;
		}
	}

	result = createProjectionFromHeaders(nitf->getHeader(),
		imageHeader.get());
	if (traceDebug())
	{
		rspfString coordinateSystem   = imageHeader->getCoordinateSystem();
		rspfNotify(rspfNotifyLevel_DEBUG)
			<< MODULE << " DEBUG:"
			<< "\ncoordinateSysetm:       " << coordinateSystem
			<< std::endl;
	}

	return result;
}


rspfProjection*
rspfNitfProjectionFactory::createProjection(const rspfKeywordlist& /* kwl */,
											const char* /* prefix */) const
{
	return 0;
}

rspfProjection*
rspfNitfProjectionFactory::createProjection(const rspfString& /* name */) const
{
	return 0;
}

rspfObject*
rspfNitfProjectionFactory::createObject(const rspfString& typeName)const
{
	return (rspfObject*)createProjection(typeName);
}

rspfObject*
rspfNitfProjectionFactory::createObject(const rspfKeywordlist& kwl,
										const char* prefix)const
{
	return createProjection(kwl, prefix);
}

void rspfNitfProjectionFactory::getTypeNameList(std::vector<rspfString>& /* typeList */)const
{

}

rspfProjection* rspfNitfProjectionFactory::createProjection(rspfImageHandler* handler)const
{
	rspfNitfTileSource* nitfTileSource = dynamic_cast<rspfNitfTileSource*> (handler);
	rspfProjection* result = 0;
	if(nitfTileSource)
	{
		if(!result)
		{
			rspfNitfImageHeader* imageHeader = nitfTileSource->getCurrentImageHeader();
			if(imageHeader)
			{
				result = createProjectionFromHeaders(nitfTileSource->getFileHeader(),imageHeader);
			}
		}
	}
	else if(isNitf(handler->getFilename()))
	{
		result =  createProjection(handler->getFilename(), handler->getCurrentEntry());
	}
	return result;
}

bool rspfNitfProjectionFactory::isNitf(const rspfFilename& filename)const
{
	std::ifstream in(filename.c_str(), ios::in|ios::binary);

	if(in)
	{
		char nitfFile[4];
		in.read((char*)nitfFile, 4);
		rspfString s(nitfFile, nitfFile+4);
		if ( (s == "NITF") || (s == "NSIF") )
		{
			return true;
		}
	}

	return false;
}

rspfProjection* rspfNitfProjectionFactory::createProjectionFromHeaders(
	rspfNitfFileHeader* fileHeader, rspfNitfImageHeader* imageHeader)const
{
	rspfProjection* result = 0;
	rspfString version = fileHeader->getVersion();
	rspfString coordinateSystem = imageHeader->getCoordinateSystem();

	// Note in version 2.0 ICORDS with 'N' == NONE.  In 2.1 it is UTM North:
	if (coordinateSystem == "G" || coordinateSystem == "D")
	{
		result = makeGeographic(imageHeader, coordinateSystem);
	}
	else if( (coordinateSystem == "S") || (coordinateSystem == "U") || 
		( version != "02.00" && (coordinateSystem == "N") ) )
	{
		result = makeUtm(imageHeader, coordinateSystem);
	}
	return result;
}

rspfProjection* rspfNitfProjectionFactory::makeGeographic(
	const rspfNitfImageHeader* hdr,
	const rspfString& coordinateSysetm) const
{
	rspfProjection* proj = 0;

	if (hdr)
	{
		// To hold corner points.
		std::vector<rspfGpt> gpts;

		//---
		// Get the corner points.
		// 
		// Look for points from the BLOCKA tag.  This may or may not be present.
		// If present since it has six digit precision use it for the points.
		//---
		if ( getBlockaPoints(hdr, gpts) == false )
		{
			rspfString geographicLocation = hdr->getGeographicLocation();

			if ( geographicLocation.size() )
			{
				if (traceDebug())
				{
					rspfNotify(rspfNotifyLevel_DEBUG)
						<< "rspfNitfProjectionFactory::makeGeographic DEBUG:"
						<< "\ngeographicLocation: " << geographicLocation
						<< std::endl;
				}

				if (coordinateSysetm == "G")
				{
					//---
					// If coord system is G then format is:
					// Lat = ddmmssX
					//       where d is degrees and m is minutes
					//       and s is seconds and X is either N (North) or S (South).
					// Lon = dddmmssX
					//       where d is degrees and m is minutes
					//       and s is seconds and X is either N (North) or S (South).
					//---
					parseGeographicString(geographicLocation, gpts);
				}
				else if (coordinateSysetm == "D")
				{
					//---
					// If coor system is D then format is:
					// +-dd.ddd +-dd.ddd four times where + is northern hemispher and
					// - is souther hemisphere for lat and longitude
					// + is easting and - is westing.
					//---
					parseDecimalDegreesString(geographicLocation, gpts);
				}

			} // matches: if ( geographicLocation.size() )

		} // matches: if ( getBlockaPoints(hdr, gpts) == false )

		if (gpts.size() == 4)
		{
			if (!isSkewed(gpts))
			{
				proj = makeEuiDistant(hdr, gpts);
			}
			else
			{
				// Image is rotated.  Make a Bilinear.
				proj = makeBilinear(hdr, gpts);
			}
		}

		if (traceDebug() && proj)
		{
			rspfNotify(rspfNotifyLevel_DEBUG)
				<< "rspfNitfProjectionFactory::makeGeographic DEBUG:"
				<< "\nUpper left corner:   " << gpts[0]
			<< "\nUpper right corner:  " << gpts[1]
			<< "\nLower right corner:  " << gpts[2]
			<< "\nLower left corner:   " << gpts[3] << endl
				<< proj->print(rspfNotify(rspfNotifyLevel_DEBUG))
				<< std::endl;
		}

	} // matches: if (hdr)

	return proj;
}

rspfProjection* rspfNitfProjectionFactory::makeUtm(
	const rspfNitfImageHeader* hdr,
	const rspfString& coordinateSystem) const
{
	rspfProjection* proj = 0;
	if (hdr)
	{
		rspfString geographicLocation = hdr->getGeographicLocation();

		if ( geographicLocation.size() )
		{
			std::vector<rspfDpt> utmPoints;
			rspf_uint32 zone;
			rspfDpt scale;
			char hemisphere = 'N';

			bool status = true;
			if ( coordinateSystem == "U")
			{
				// Sets zone, hemisphere and utmPoints. Returns true on success.
				status = parseMgrsString(geographicLocation, zone, hemisphere, utmPoints);
			}
			else
			{
				// Sets zone and utmPoints.  Void return...
				parseUtmString(geographicLocation, zone, utmPoints);
				if(coordinateSystem == "S")
				{
					hemisphere = 'S';
				}
			}

			if ( status )
			{
				if ( traceDebug() )
				{
					std::string s;
					s.push_back(hemisphere);
					rspfNotify(rspfNotifyLevel_DEBUG)
						<< "rspfNitfProjectionFactory::makeUtm DEBUG"
						<< "\ngeo string: " << geographicLocation
						<< "\nutm zone:   " << zone
						<< "\nhemisphere: " << hemisphere
						<< std::endl;
					for (rspf_uint32 i=0; i<utmPoints.size(); ++i)
					{
						rspfNotify(rspfNotifyLevel_DEBUG)
							<< "utmPoints[" << utmPoints[i] << std::endl;
					}
				}

				rspfRefPtr<rspfUtmProjection> uproj = new rspfUtmProjection;
				uproj->setHemisphere(hemisphere);
				uproj->setZone(zone);

				if(isSkewed(utmPoints))
				{
					std::vector<rspfGpt> gpts;

					// Try blocka points first as they are more accurate.
					if ( getBlockaPoints(hdr, gpts) == false )
					{
						rspfGpt ul = uproj->inverse(utmPoints[0]);
						rspfGpt ur = uproj->inverse(utmPoints[1]);
						rspfGpt lr = uproj->inverse(utmPoints[2]);
						rspfGpt ll = uproj->inverse(utmPoints[3]);
						gpts.push_back(ul);
						gpts.push_back(ur);
						gpts.push_back(lr);
						gpts.push_back(ll);
					}

					//---
					// Make a bilinear either from our skewed utm points or the points from the blocka
					// tag.
					//---
					proj = makeBilinear(hdr, gpts);

					uproj = 0; // Done with utm projeciton

				}
				else
				{
					computeScaleInMeters(hdr, utmPoints, scale);

					//---
					// Assign our projection to the return "proj".
					// Use rspfRefPtr::release the so we don't delete proj when uproj
					// goes out of scope.
					//---
					proj = uproj.release(); 
				}

				if( scale.hasNans() == false )
				{
					//---
					// Get the tie point.
					// 
					// Look for the the BLOCKA tag which may or may not be present.
					// This has six digit precision in decimal degrees which equates to
					// about 0.11 meters (at equator) as compared to 1.0 accuaracy of the
					// IGEOLO field.
					//---
					rspfDpt tie;
					std::vector<rspfGpt> gpts;
					if ( getBlockaPoints(hdr, gpts) )
					{
						if (traceDebug())
						{
							rspfNotify(rspfNotifyLevel_DEBUG)
								<< "rspfNitfProjectionFactory::makeUtm DEBUG:"
								<< "\nTie point from blocka: " << gpts[0]
							<< endl;
						}

						tie = proj->forward(gpts[0]);
						tie.x += scale.x/2.0;
						tie.y -= scale.y/2.0;
					}
					else
					{
						tie.x = utmPoints[0].x + scale.x/2.0;
						tie.y = utmPoints[0].y - scale.y/2.0;
					}

					if (traceDebug())
					{
						rspfNotify(rspfNotifyLevel_DEBUG)
							<< "rspfNitfProjectionFactory::makeUtm DEBUG:"
							<< "\nTie point: " << tie
							<< "\nScale:     " << scale
							<< endl;
					}

					// Set the tie and scale.
					rspfMapProjection* mproj = dynamic_cast<rspfMapProjection*>(proj);
					if ( mproj )
					{
						mproj->setUlEastingNorthing(tie);
						mproj->setMetersPerPixel(scale);
					}
					else // cannot cast
					{
						if ( proj )
						{
							delete proj;
							proj = 0;
						}
					}
				}
				else // Scale has nans
				{
					if ( proj )
					{
						delete proj;
						proj = 0;
					}
				}

			} // matches: if (status)

		} // matches:  if ( geographicLocation.size() )

	} // matches: if (hdr)

	return proj;
}

bool rspfNitfProjectionFactory::parseMgrsString(const rspfString& mgrsLocationString,
												rspf_uint32& zone,
												char& hemisphere,
												std::vector<rspfDpt>& utmPoints)const
{
	bool result = false; // Start false.

	//---
	// From spec:
	// UTM expressed in MGRS use the format zzBJKeeeeennnnn (15 characters).
	// Assumption Zone and hemisphere same for all corners.
	//---
	if ( mgrsLocationString.size() >= 60 )
	{
		// Split the location string into four separate ones.
		std::vector<std::string> mgrsStr(4); // Corner strings.
		mgrsStr[0] = mgrsLocationString.substr(0, 15);
		mgrsStr[1] = mgrsLocationString.substr(15, 15);
		mgrsStr[2] = mgrsLocationString.substr(30, 15);
		mgrsStr[3] = mgrsLocationString.substr(45, 15);

		utmPoints.resize(4);
		long z = 0;
		rspf_float64 e=0.0;
		rspf_float64 n=0.0;

		result = true; // Set to true.

		//---
		// Convert each string to Easting Northing.  This also sets zone hemisphere.
		// Method takes long for zone.
		//---
		for (rspf_uint32 i = 0; i < 4; ++i)
		{
			if ( Convert_RSPF_MGRS_To_UTM(mgrsStr[i].c_str(), &z, &hemisphere, &e, &n) == 0 )
			{
				utmPoints[i].x = e;
				utmPoints[i].y = n;
			}
			else
			{
				result = false; // Geotrans code errored on string.
				break;
			}
		}
		if (result) zone = static_cast<rspf_uint32>(z); // Set the zone.
	}
	return result;
}

rspfProjection* rspfNitfProjectionFactory::makeEuiDistant(
	const rspfNitfImageHeader* hdr,
	const std::vector<rspfGpt>& gpts) const
{
	rspfEquDistCylProjection* proj = 0;

	// Get the scale.
	rspfDpt scale;
	computeScaleInDecimalDegrees(hdr, gpts, scale);

	if (scale.hasNans())
	{
		return proj;
	}

	// Make the projection.
	proj = new rspfEquDistCylProjection();

	if ( scale.x )
	{
		rspfGpt origin = proj->getOrigin();

		//---
		// rspfEquDistCylProjection uses the origin_latitude for meters per pixel (gsd)
		// computation.  So is not set in tiff tags, compute to achieve the proper
		// horizontal scaling.
		//---
		origin.lat = rspf::acosd(scale.y/scale.x);

		proj->setOrigin(origin);
	}

	// Shift the tie to the center of the pixel.
	rspfGpt tiePoint;
	tiePoint.latd(gpts[0].latd() - (scale.y/2.0));
	tiePoint.lond(gpts[0].lond() + (scale.x/2.0));

	// Set the tie points.
	proj->setUlTiePoints(tiePoint);

	// Set the scale.
	proj->setDecimalDegreesPerPixel(scale);

	return proj;
}

rspfProjection* rspfNitfProjectionFactory::makeBilinear(
	const rspfNitfImageHeader* hdr,
	const std::vector<rspfGpt>& gpts) const
{
	double rows = hdr->getNumberOfRows();
	double cols = hdr->getNumberOfCols();

	rspfDpt ul(0.0, 0.0);
	rspfDpt ur(cols-1.0, 0.0);   
	rspfDpt lr(cols-1.0, rows-1.0);
	rspfDpt ll(0.0, rows-1.0);

	rspfRefPtr<rspfBilinearProjection> proj;
	try
	{
		proj = new rspfBilinearProjection(ul,
			ur,
			lr,
			ll,
			gpts[0],
			gpts[1],
			gpts[2],
			gpts[3]);
	}
	catch(...)
	{
		proj = 0;
	}

	return proj.release();
}

bool rspfNitfProjectionFactory::isSkewed(
	const std::vector<rspfGpt>& gpts) const
{

	return !( (gpts[0].latd() == gpts[1].latd()) &&
		(gpts[2].latd() == gpts[3].latd()) &&
		(gpts[0].lond() == gpts[3].lond()) &&
		(gpts[1].lond() == gpts[2].lond()) );

}

bool rspfNitfProjectionFactory::isSkewed(
	const std::vector<rspfDpt>& dpts) const
{
	return !( (dpts[0].y == dpts[1].y) &&
		(dpts[2].y == dpts[3].y) &&
		(dpts[0].x == dpts[3].x) &&
		(dpts[1].x == dpts[2].x) );

}

bool rspfNitfProjectionFactory::getBlockaPoints(
	const rspfNitfImageHeader* hdr,
	std::vector<rspfGpt>& gpts) const
{
	if (!hdr)
	{
		return false;
	}

	rspfRefPtr<rspfNitfRegisteredTag> tag =
		hdr->getTagData(rspfString("BLOCKA"));

	if (!tag)
	{
		if (traceDebug())
		{
			rspfNotify(rspfNotifyLevel_DEBUG)
				<< "rspfNitfProjectionFactory::getBlockaPoints DEBUG:"
				<< "\nBLOCKA tag not found."
				<< std::endl;
		}
		return false;
	}

	if (gpts.size())
	{
		gpts.clear();
	}

	rspfNitfBlockaTag* blockaTag = PTR_CAST(rspfNitfBlockaTag, tag.get());
	if (!blockaTag)
	{
		return false;
	}

	rspfDpt dpt;
	rspfGpt gpt;

	// Get the upper left or first row first column.
	blockaTag->getFrfcLoc(dpt);
	gpt.latd(dpt.y);
	gpt.lond(dpt.x);
	gpts.push_back(gpt);

	// Get the upper right or first row last column.
	blockaTag->getFrlcLoc(dpt);
	gpt.latd(dpt.y);
	gpt.lond(dpt.x);
	gpts.push_back(gpt);

	// Get the lower right or last row last column.
	blockaTag->getLrlcLoc(dpt);
	gpt.latd(dpt.y);
	gpt.lond(dpt.x);
	gpts.push_back(gpt);

	// Get the lower left or last row first column.
	blockaTag->getLrfcLoc(dpt);
	gpt.latd(dpt.y);
	gpt.lond(dpt.x);
	gpts.push_back(gpt);

	if (traceDebug())
	{
		rspfNotify(rspfNotifyLevel_DEBUG)
			<< "rspfNitfProjectionFactory::getBlockaPoints DEBUG:"
			<< std::endl;
		for (int i=0; i<4; ++i)
		{
			rspfNotify(rspfNotifyLevel_DEBUG)
				<< "gpt[" << i << "] " << gpts[i] << std::endl;
		}
	}

	return true;
}

void rspfNitfProjectionFactory::computeScaleInDecimalDegrees(
	const rspfNitfImageHeader* hdr,
	const std::vector<rspfGpt>& gpts,
	rspfDpt& scale) const
{
	if ( !hdr || isSkewed(gpts))
	{
		scale.makeNan();
		return;
	}
	rspfIrect imageRect = hdr->getImageRect();

	//---
	// Calculate the scale.  This assumes that the corner points are for the
	// edge of the corner pixels, not the center of the corner pixels.
	//---
	double longitudeSize  = 0.0;
	double latitudeSize = 0.0;
	if ( (gpts[1].lond() < 0.0) && (gpts[0].lond() >= 0) )
	{
		//---
		// Upper right negative(Western), upper left positive (Eastern).
		// Crossing date line maybe???
		//---
		longitudeSize = (gpts[1].lond() + 360.0) - gpts[0].lond();
	}
	else
	{
		longitudeSize = gpts[1].lond() - gpts[0].lond();
	}

	latitudeSize = gpts[0].latd() - gpts[2].latd();

	double rows = imageRect.height();
	double cols = imageRect.width();
	//   double rows = hdr->getNumberOfRows();
	//   double cols = hdr->getNumberOfCols();

	if (!rows || !cols)
	{
		scale.makeNan();
		return;
	}
	scale.y = latitudeSize  / rows;
	scale.x = longitudeSize / cols;
}

void rspfNitfProjectionFactory::computeScaleInMeters(
	const rspfNitfImageHeader* hdr,
	const std::vector<rspfDpt>& dpts,
	rspfDpt& scale) const
{
	if ( !hdr || isSkewed(dpts))
	{
		scale.makeNan();
		return;
	}
	rspfIrect imageRect = hdr->getImageRect();

	//---
	// Calculate the scale.  This assumes that the corner points are for the
	// edge of the corner pixels, not the center of the corner pixels.
	//---
	double eastingSize  = 0.0;
	double northingSize = 0.0;

	eastingSize  = fabs(dpts[1].x - dpts[0].x);
	northingSize = fabs(dpts[0].y - dpts[3].y);

	double rows = imageRect.height();//hdr->getNumberOfRows();
	double cols = imageRect.width();//hdr->getNumberOfCols();

	if (!rows || !cols)
	{
		scale.makeNan();
		return;
	}
	scale.y = northingSize / rows;
	scale.x = eastingSize  / cols;
}

void rspfNitfProjectionFactory::parseUtmString(const rspfString& utmLocation,
											   rspf_uint32& zone,
											   std::vector<rspfDpt>& utmPoints)const
{
	rspf_uint32 idx = 0;
	rspfString z;
	rspfString east;
	rspfString north;


	z    = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 2);
	idx += 2;
	east = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 6);
	idx += 6;
	north = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 7);
	idx += 7;

	utmPoints.push_back(rspfDpt(east.toDouble(),
		north.toDouble()));

	z    = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 2);
	idx += 2;
	east = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 6);
	idx += 6;
	north = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 7);
	idx += 7;
	utmPoints.push_back(rspfDpt(east.toDouble(),
		north.toDouble()));

	z    = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 2);
	idx += 2;
	east = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 6);
	idx += 6;
	north = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 7);
	idx += 7;
	utmPoints.push_back(rspfDpt(east.toDouble(),
		north.toDouble()));

	z    = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 2);
	idx += 2;
	east = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 6);
	idx += 6;
	north = rspfString(utmLocation.begin() + idx,
		utmLocation.begin() + idx + 7);
	idx += 7;
	utmPoints.push_back(rspfDpt(east.toDouble(),
		north.toDouble()));

	zone = z.toUInt32();
}


void rspfNitfProjectionFactory::parseGeographicString(
	const rspfString& geographicLocation, std::vector<rspfGpt>& gpts) const
{
	gpts.clear();

	if (geographicLocation.size() != 60)
	{
		return;
	}

	std::string::size_type geo_index = 0;
	for (int i=0; i<4; ++i)
	{
		//---
		// We have to split up the geographicLocation string for the dms class.
		// 
		// geographicLocation = ddmmssXdddmmssX (four times).
		// "dd mm ss X" has a string length of 10
		// "ddd mm ss X" has a string length of 11
		//---
		std::string::size_type lat_index = 0;
		std::string::size_type lon_index = 0;
		const char SPACE = ' ';
		rspfString latString(10, SPACE);
		rspfString lonString(11, SPACE);

		// degrees latitude
		latString[lat_index++] = geographicLocation[geo_index++];
		latString[lat_index++] = geographicLocation[geo_index++];
		++lat_index;

		// minutes latitude
		latString[lat_index++] = geographicLocation[geo_index++];
		latString[lat_index++] = geographicLocation[geo_index++];
		++lat_index;

		// seconds latitude
		latString[lat_index++] = geographicLocation[geo_index++];
		latString[lat_index++] = geographicLocation[geo_index++];
		++lat_index;

		// hemisphere
		latString[lat_index++] = geographicLocation[geo_index++];

		// degrees longitude
		lonString[lon_index++] = geographicLocation[geo_index++];
		lonString[lon_index++] = geographicLocation[geo_index++];
		lonString[lon_index++] = geographicLocation[geo_index++];
		++lon_index;

		// minutes longitude
		lonString[lon_index++] = geographicLocation[geo_index++];
		lonString[lon_index++] = geographicLocation[geo_index++];
		++lon_index;

		// seconds longitude
		lonString[lon_index++] = geographicLocation[geo_index++];
		lonString[lon_index++] = geographicLocation[geo_index++];
		++lon_index;

		// hemisphere
		lonString[lon_index++] = geographicLocation[geo_index++];

		// Convert to decimal degrees using the dms class.
		rspfGpt gpt;
		rspfDms dms(0.0);
		dms.setLatFlag(true);
		if ( ! dms.setDegrees(latString.c_str()) )
		{
			gpts.clear();
			return;
		}
		gpt.latd(dms.getDegrees());

		dms.setLatFlag(false);
		if ( ! dms.setDegrees(lonString.c_str()) )
		{
			gpts.clear();
			return;
		}
		gpt.lond(dms.getDegrees());

		gpts.push_back(gpt);

		if (traceDebug())
		{
			rspfNotify(rspfNotifyLevel_DEBUG)
				<< "rspfNitfProjectionFactory::parseGeographicString DEBUG:"
				<< "\nground point[" << i << "]:  " << gpt
				<< std::endl;
		}
	}
}

void rspfNitfProjectionFactory::parseDecimalDegreesString(const rspfString& geographicLocation,
														  std::vector<rspfGpt>& gpts) const
{
	const char* bufPtr = geographicLocation.c_str();


	rspfString ulLat(bufPtr,
		bufPtr + 7);
	bufPtr+=7;
	rspfString ulLon(bufPtr,
		bufPtr+8);
	bufPtr+=8;
	rspfString urLat(bufPtr,
		bufPtr + 7);
	bufPtr+=7;
	rspfString urLon(bufPtr,
		bufPtr+8);
	bufPtr+=8;
	rspfString lrLat(bufPtr,
		bufPtr + 7);
	bufPtr+=7;
	rspfString lrLon(bufPtr,
		bufPtr+8);
	bufPtr+=8;
	rspfString llLat(bufPtr,
		bufPtr + 7);
	bufPtr+=7;
	rspfString llLon(bufPtr,
		bufPtr+8);

	gpts.push_back(rspfGpt(ulLat.toDouble(), ulLon.toDouble()));
	gpts.push_back(rspfGpt(urLat.toDouble(), urLon.toDouble()));
	gpts.push_back(rspfGpt(lrLat.toDouble(), lrLon.toDouble()));
	gpts.push_back(rspfGpt(llLat.toDouble(), llLon.toDouble()));

	if (traceDebug())
	{
		rspfNotify(rspfNotifyLevel_DEBUG)
			<< "rspfNitfProjectionFactory::parseDecimalDegreesString DEBUG:"
			<< "\nground point[" << 0 << "]:  " << gpts[0]
		<< "\nground point[" << 1 << "]:  " << gpts[1]
		<< "\nground point[" << 2 << "]:  " << gpts[2]
		<< "\nground point[" << 3 << "]:  " << gpts[3]
		<< std::endl;
	}
}

rspfNitfProjectionFactory::rspfNitfProjectionFactory(const rspfNitfProjectionFactory&)
{
}

rspfNitfProjectionFactory& rspfNitfProjectionFactory::operator=(const rspfNitfProjectionFactory&)
{
	return *this;
}
