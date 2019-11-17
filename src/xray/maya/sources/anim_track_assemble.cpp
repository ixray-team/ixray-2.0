#include "pch.h"
#include <xray/maya_animation/anim_track.h>

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "anim_export_util.h"

#define kMaxTan 5729577.9485111479f
using namespace xray::maya_animation;
//using namespace xray::math;

/*
//	Function Name:
//		assembleAnimCurve
//
//	Description:
//		A static helper function to assemble an EtCurve animation curve
//	from a linked list of heavy-weights keys
//
//  Input Arguments:
//		EtReadKey *firstKey			The linked list of keys
//		int numKeys				The number of keyss
//		bool isWeighted		Whether or not the curve has weighted tangents
//		bool useOldSmooth		Whether or not to use pre-Maya2.0 smooth
//									tangent computation
//
//  Return Value:
//		EtCurve *animCurve			The assembled animation curve
//
//	Note:
//		This function will also free the memory used by the heavy-weight keys
*/
;

int assembleAnimCurve (EtReadKey* firstKey, int numKeys, bool isWeighted, EtCurve* src_and_dest)
{
	EtReadKey*	prevKey			= 0;
	EtReadKey*	key				= 0;
	EtReadKey*	nextKey			= 0;
	int		index;
	EtKey*		thisKey;
	float		py, ny, dx;
	bool	hasSmooth;
	float		length;
	float		inTanX = xray::memory::uninitialized_value<float>(), inTanY = xray::memory::uninitialized_value<float>(), outTanX = xray::memory::uninitialized_value<float>(), outTanY = xray::memory::uninitialized_value<float>();
	float		inTanXs, inTanYs, outTanXs, outTanYs;

	// make sure we have useful information 
	if ((firstKey == 0) || (numKeys == 0)) 
		return (0);

	/* allocate some memory for the animation curve */
	src_and_dest->create_keys	(numKeys);

	/* initialise the animation curve parameters */
	src_and_dest->numKeys		= numKeys;
	src_and_dest->isWeighted	= isWeighted;
	src_and_dest->isStatic		= true;
	src_and_dest->preInfinity	= kInfinityConstant;
	src_and_dest->postInfinity	= kInfinityConstant;

	/* initialise the cache */
	src_and_dest->lastKey		= 0;
	src_and_dest->lastIndex		= -1;
	src_and_dest->lastInterval	= -1;
	src_and_dest->isStep		= false;
	src_and_dest->isStepNext	= false;

	/* compute tangents */
	nextKey			= firstKey;
	index			= 0;
	while(nextKey != 0) 
	{
		if (prevKey != 0) 
			FREE(prevKey);

		prevKey		= key;
		key			= nextKey;
		nextKey		= nextKey->next;

		/* construct the final EtKey (light-weight key) */
		thisKey			= &(src_and_dest->keyListPtr[index++]);
		thisKey->time	= key->time;
		thisKey->value	= key->value;

		/* compute the in-tangent values */
		/* kTangentClamped */
		if ((key->inTangentType == MFnAnimCurve::kTangentClamped) && (prevKey != 0)) 
		{
			py = prevKey->value - key->value;
			if (py < 0.0) 
				py = -py;

			ny = (nextKey == 0 ? py : nextKey->value - key->value);
			
			if (ny < 0.0) 
				ny = -ny;

			if ((ny <= 0.05) || (py <= 0.05)) 
				key->inTangentType = MFnAnimCurve::kTangentFlat;
		}
		hasSmooth	= false;
		switch (key->inTangentType) 
		{
		case MFnAnimCurve::kTangentFixed:
			inTanX		= key->inWeightX * cos (key->inAngle) * 3.0f ;
			inTanY		= key->inWeightY * sin (key->inAngle) * 3.0f ;
			break;

		case MFnAnimCurve::kTangentLinear:
			if (prevKey == 0) 
			{
				inTanX	= 1.0f;
				inTanY	= 0.0f;
			}else 
			{
				inTanX	= key->time - prevKey->time;
				inTanY	= key->value - prevKey->value;
			}
			break;

		case MFnAnimCurve::kTangentFlat:
			if (prevKey == 0) 
			{
				inTanX	= (nextKey == 0 ? 0.0f : nextKey->time - key->time);
				inTanY	= 0.0f;
			}else 
			{
				inTanX	= key->time - prevKey->time;
				inTanY	= 0.0f;
			}
			break;

		case MFnAnimCurve::kTangentStep:
			inTanX		= 0.0f;
			inTanY		= 0.0f;
			break;

		case MFnAnimCurve::kTangentStepNext:
			inTanX		= FLT_MAX;
			inTanY		= FLT_MAX;
			break;

		case MFnAnimCurve::kTangentSlow:
		case MFnAnimCurve::kTangentFast:
			key->inTangentType = MFnAnimCurve::kTangentSmooth;
			if (prevKey == 0) 
			{
				inTanX	= 1.0;
				inTanY	= 0.0;
			}else 
			{
				inTanX	= key->time - prevKey->time;
				inTanY	= key->value - prevKey->value;
			}
			break;

		case MFnAnimCurve::kTangentSmooth:
		case MFnAnimCurve::kTangentClamped:
			key->inTangentType	= MFnAnimCurve::kTangentSmooth;
			hasSmooth			= true;
			break;
		}

		/* compute the out-tangent values */
		/* kTangentClamped */
		if ((key->outTangentType == MFnAnimCurve::kTangentClamped) && (nextKey != 0)) 
		{
			ny = nextKey->value - key->value;
			if (ny < 0.0f) 
				ny = -ny;

			py = (prevKey == 0 ? ny : prevKey->value - key->value);
			
			if (py < 0.0f) 
				py = -py;

			if ((ny <= 0.05f) || (py <= 0.05f)) 
				key->outTangentType = MFnAnimCurve::kTangentFlat;
		}
		switch (key->outTangentType) 
		{
		case MFnAnimCurve::kTangentFixed:
			outTanX 		= key->outWeightX * cos (key->outAngle) * 3.0f ;
			outTanY 		= key->outWeightY * sin (key->outAngle) * 3.0f ;
			break;

		case MFnAnimCurve::kTangentLinear:
			if (nextKey == 0) 
			{
				outTanX 	= 1.0f;
				outTanY 	= 0.0f;
			}
			else {
				outTanX		= nextKey->time - key->time;
				outTanY		= nextKey->value - key->value;
			}
			break;

		case MFnAnimCurve::kTangentFlat:
			if (nextKey == 0) 
			{
				outTanX		= (prevKey == 0 ? 0.0f : key->time - prevKey->time);
				outTanY		= 0.0f;
			}else 
			{
				outTanX		= nextKey->time - key->time;
				outTanY		= 0.0f;
			}
			break;

		case MFnAnimCurve::kTangentStep:
			outTanX			= 0.0f;
			outTanY			= 0.0f;
			break;

		case MFnAnimCurve::kTangentStepNext:
			outTanX			= FLT_MAX;
			outTanY			= FLT_MAX;
			break;

		case MFnAnimCurve::kTangentSlow:
		case MFnAnimCurve::kTangentFast:
			key->outTangentType = MFnAnimCurve::kTangentSmooth;
			if (nextKey == 0) 
			{
				outTanX		= 1.0;
				outTanY		= 0.0;
			}
			else {
				outTanX		= nextKey->time - key->time;
				outTanY		= nextKey->value - key->value;
			}
			break;

		case MFnAnimCurve::kTangentSmooth:
		case MFnAnimCurve::kTangentClamped:
			key->outTangentType = MFnAnimCurve::kTangentSmooth;
			hasSmooth			= true;
			break;
		}

		/* compute smooth tangents (if necessary) */
		if (hasSmooth) 
		{
			{
				/* Maya 2.0 smooth tangents */
				if ((prevKey == 0) && (nextKey != 0)) 
				{
					outTanXs	= nextKey->time - key->time;
					outTanYs	= nextKey->value - key->value;
					inTanXs		= outTanXs;
					inTanYs		= outTanYs;
				}else 
				if ((prevKey != 0) && (nextKey == 0)) 
				{
					outTanXs	= key->time - prevKey->time;
					outTanYs	= key->value - prevKey->value;
					inTanXs		= outTanXs;
					inTanYs		= outTanYs;
				}else 
				if ((prevKey != 0) && (nextKey != 0)) 
				{
					/* There is a CV before and after this one*/
					/* Find average of the adjacent in and out tangents. */

					dx = nextKey->time - prevKey->time;
					if (dx < 0.0001) 
						outTanYs = kMaxTan;
					else 
						outTanYs = (nextKey->value - prevKey->value) / dx;

					outTanXs		= nextKey->time - key->time;
					inTanXs			= key->time - prevKey->time;
					inTanYs			= outTanYs * inTanXs;
					outTanYs		*= outTanXs;
				}else 
				{
					inTanXs			= 1.0;
					inTanYs			= 0.0;
					outTanXs		= 1.0;
					outTanYs		= 0.0;
				}
			}

			if (key->inTangentType == MFnAnimCurve::kTangentSmooth) 
			{
				inTanX = inTanXs;
				inTanY = inTanYs;
			}
			if (key->outTangentType == MFnAnimCurve::kTangentSmooth) 
			{
				outTanX = outTanXs;
				outTanY = outTanYs;
			}
		}

		/* make sure the computed tangents are valid */
		if (src_and_dest->isWeighted) 
		{
			if (inTanX < 0.0) 
				inTanX = 0.0;

			if (outTanX < 0.0) 
				outTanX = 0.0;
		}else 
		if (( inTanX == FLT_MAX && inTanY == FLT_MAX ) 
			|| ( outTanX == FLT_MAX && outTanY == FLT_MAX ) )
		{
			// SPecial case for step next tangents, do nothing
		}else 
		{
			if (inTanX < 0.0)
				inTanX = 0.0;
			
			length			= sqrt ((inTanX * inTanX) + (inTanY * inTanY));
			if (length != 0.0f) 
			{	/* zero lengths can come from step tangents */
				inTanX /= length;
				inTanY /= length;
			}
			if ((inTanX == 0.0f) && (inTanY != 0.0f)) 
			{
				inTanX = 0.0001f;
				inTanY = (inTanY < 0.0f ? -1.0f : 1.0f) * (inTanX * kMaxTan);
			}
			if (outTanX < 0.0f) 
			{
				outTanX = 0.0f;
			}
			
			length				= sqrt ((outTanX * outTanX) + (outTanY * outTanY));
			if (length != 0.0f) 
			{	/* zero lengths can come from step tangents */
				outTanX /= length;
				outTanY /= length;
			}
			if ((outTanX == 0.0f) && (outTanY != 0.0f)) 
			{
				outTanX = 0.0001f;
				outTanY = (outTanY < 0.0f ? -1.0f : 1.0f) * (outTanX * kMaxTan);
			}
		}

		thisKey->inTanX 		= inTanX;
		thisKey->inTanY 		= inTanY;
		thisKey->outTanX		= outTanX;
		thisKey->outTanY		= outTanY;

		/*
		// check whether or not this animation curve is static (i.e. all the
		// key values are the same)
		*/
		if (src_and_dest->isStatic) 
		{
			if ((prevKey != 0) && (prevKey->value != key->value)) 
			{
				src_and_dest->isStatic = false;
			}else 
			if ((inTanY != 0.0f) || (outTanY != 0.0f)) 
			{
				src_and_dest->isStatic = false;
			}
		}
	}
	if (src_and_dest->isStatic) 
	{
		if ((prevKey != 0) && (key != 0) && (prevKey->value != key->value)) 
			src_and_dest->isStatic = false;
	}
	if (prevKey != 0) 
		FREE(prevKey);

	if (key != 0) 
		FREE(key);

	return 0;
}
