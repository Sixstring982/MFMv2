/* -*- C++ -*- */

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) > (Y) ? (Y) : (X))

template <class T, u32 R>
u32 TileRenderer::GetAtomColor(Tile<T,R>& tile, T& atom)
{
  switch(tile.GetStateFunc()(&atom))
  {
  case ELEMENT_DREG:
    return 0xff505050;
  case ELEMENT_RES:
    return 0xffffff00;
  case ELEMENT_SORTER:
    return 0xffff0000;
  case ELEMENT_EMITTER:
    return 0xff808080;
  case ELEMENT_DATA:
    return 0xff0000ff;
  case ELEMENT_CONSUMER:
    return 0xff101010;
  }
  return 0;
}

template <class T, u32 R>
u32 TileRenderer::GetDataHeatColor(Tile<T,R>& tile, T& atom)
{
  if(tile.GetStateFunc()(&atom) == ELEMENT_DATA)
  {
    u8 tcl = atom.ReadLowerBits() << 1;
    return 0xff0000ff | (tcl << 8);
  }
  if(tile.GetStateFunc()(&atom) == ELEMENT_SORTER)
  {
    u8 tcl = atom.ReadLowerBits() << 1;
    return 0xffff0000 | (tcl << 8);
  }
  return 0;
}


template <class T,u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderAtoms(Point<int>& pt, Tile<T,EVENT_WINDOW_RADIUS>& tile,
			       bool renderCache)
{
  u32 astart = renderCache ? 0 : EVENT_WINDOW_RADIUS;
  u32 aend   = renderCache ? TILE_WIDTH : TILE_WIDTH - EVENT_WINDOW_RADIUS;

  u32 cacheOffset = renderCache ? 0 : -EVENT_WINDOW_RADIUS * m_atomDrawSize;

  Point<int> atomLoc;

  Point<u32> rendPt;

  for(u32 x = astart; x < aend; x++)
  {
    rendPt.SetX(pt.GetX() + m_atomDrawSize * x +
		m_windowTL.GetX() + cacheOffset);
    if(rendPt.GetX() + m_atomDrawSize < m_dimensions.GetX())
    {
      atomLoc.SetX(x);
      for(u32 y = astart; y < aend; y++)
      {
	rendPt.SetY(pt.GetY() + m_atomDrawSize * y +
		    m_windowTL.GetY() + cacheOffset);
	if(rendPt.GetY() + m_atomDrawSize < m_dimensions.GetY())
	{
	  atomLoc.SetY(y);

	  T* atom = tile.GetAtom(&atomLoc);
	  u32 color;
	  if(m_drawDataHeat)
	  {
	    color = GetDataHeatColor(tile, *atom);
	  }
	  else
	  {
	    color = GetAtomColor(tile, *atom);
	  }
			  
	  
	  if(rendPt.GetX() + m_atomDrawSize < m_dimensions.GetX() &&
	     rendPt.GetY() + m_atomDrawSize < m_dimensions.GetY())
	  {
	    if(color)
	    {
	      Drawing::FillCircle(m_dest,
				  rendPt.GetX(),
				  rendPt.GetY(),
				  m_atomDrawSize,
				  m_atomDrawSize,
				  (m_atomDrawSize / 2) - 2,
				  color);
	    }
	  }
	}
      }
    }
  }
}

template <class T,u32 R>
void TileRenderer::RenderTile(Tile<T,R>& t, Point<int>& loc, bool renderWindow,
			      bool renderCache)
{
  Point<int> multPt(loc);

  multPt.Multiply((TILE_WIDTH - R * 2) * 
		  m_atomDrawSize);

  Point<int> realPt(multPt.GetX(), multPt.GetY());


  u32 tileHeight = TILE_WIDTH * m_atomDrawSize;

  realPt.Add(m_windowTL);

  if(realPt.GetX() + tileHeight >= 0 &&
     realPt.GetY() + tileHeight >= 0 &&
     realPt.GetX() < (s32)m_dest->w &&
     realPt.GetY() < (s32)m_dest->h)
  {
    if(m_drawMemRegions)
    {
      RenderMemRegions<R>(multPt, renderCache);
    }

    if(renderWindow)
    {
      RenderEventWindow(multPt, t, renderCache);
    }

    RenderAtoms(multPt, t, renderCache);
    
    if(m_drawGrid)
    {
      RenderGrid<R>(&multPt, renderCache);
    }
  }  
}

template <class T,u32 R>
void TileRenderer::RenderEventWindow(Point<int>& offset,
				     Tile<T,R>& tile, bool renderCache)
{
  Point<int> winCenter;
  tile.FillLastExecutedAtom(winCenter);

  Point<int> atomLoc;
  Point<int> eventCenter;
  u32 cacheOffset = renderCache ? 0 : -R;
  u32 drawColor = Drawing::WHITE;
  
  tile.FillLastExecutedAtom(eventCenter);
  u32 tableSize = EVENT_WINDOW_SITES(R);
  for(u32 i = 0; i < tableSize; i++)
  {
    ManhattanDir<R>::get().FillFromBits(atomLoc, i, (TableType)R);
    atomLoc.Add(eventCenter);
    atomLoc.Add(cacheOffset, cacheOffset);

    if(i == 0) // Center atom first in indexing.
    {
      drawColor = Drawing::GREEN;
    }
    else
    {
      drawColor = Drawing::WHITE;
    }

    RenderAtomBG(offset, atomLoc, drawColor);
  }
}

template <u32 R>
void TileRenderer::RenderMemRegions(Point<int>& pt, bool renderCache)
{
  int regID = 0;
  if(renderCache)
  {
    RenderMemRegion<R>(pt, regID++, m_cacheColor, renderCache);
  }
  RenderMemRegion<R>(pt, regID++, m_sharedColor, renderCache);
  RenderMemRegion<R>(pt, regID++, m_visibleColor, renderCache);
  RenderMemRegion<R>(pt, regID  , m_hiddenColor, renderCache);
}

template <u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderMemRegion(Point<int>& pt, int regID,
				   Uint32 color, bool renderCache)
{
  int tileSize;
  if(!renderCache)
  {
    /* Subtract out the cache's width */
    tileSize = m_atomDrawSize * 
      (TILE_WIDTH - 2 * EVENT_WINDOW_RADIUS);
  }
  else
  {
    tileSize = m_atomDrawSize * TILE_WIDTH;
  }

  int ewrSize = EVENT_WINDOW_RADIUS * m_atomDrawSize;

  /* Manually fill the rect so we can stop at the right place. */
  Point<s32> topPt(pt.GetX() + (ewrSize * regID) + m_windowTL.GetX(),
		   pt.GetY() + (ewrSize * regID) + m_windowTL.GetY());
  Point<s32> botPt(MIN((s32)m_dimensions.GetX(), topPt.GetX() + (tileSize - (ewrSize * regID * 2))),
		   MIN((s32)m_dimensions.GetY(), topPt.GetY() + (tileSize - (ewrSize * regID * 2))));
  for(s32 x = topPt.GetX(); x < botPt.GetX(); x++)
  {
    for(s32 y = topPt.GetY(); y < botPt.GetY(); y++)
    {
      Drawing::SetPixel(m_dest, x, y, color);
    }
  }
}

template <u32 EVENT_WINDOW_RADIUS>
void TileRenderer::RenderGrid(Point<int>* pt, bool renderCache)
{
  s32 lineLen, linesToDraw;

  if(!renderCache)
  {
    lineLen = m_atomDrawSize * 
      (TILE_WIDTH - 2 * EVENT_WINDOW_RADIUS);
    linesToDraw = TILE_WIDTH + 
      1 - (2 * EVENT_WINDOW_RADIUS);
  }
  else
  {
    lineLen = m_atomDrawSize * TILE_WIDTH;
    linesToDraw = TILE_WIDTH + 1;
  }

  s32 lowBound =
    MIN((s32)m_dimensions.GetY(), pt->GetY() + m_windowTL.GetY() + lineLen);

  for(int x = 0; x < linesToDraw; x++)
  {
    if(pt->GetX() + x * m_atomDrawSize + m_windowTL.GetX() > (s32)m_dimensions.GetX())
    {
      break;
    }
    Drawing::DrawVLine(m_dest,
		       pt->GetX() + x * m_atomDrawSize +
		       m_windowTL.GetX(),
		       pt->GetY() + m_windowTL.GetY(),
		       lowBound,
		       m_gridColor);
  }

  s32 rightBound =
    MIN((s32)m_dimensions.GetX(), pt->GetX() + m_windowTL.GetX() + lineLen);

  for(int y = 0; y < linesToDraw; y++)
  {
    if(pt->GetY() + y * m_atomDrawSize + m_windowTL.GetY() > (s32)m_dimensions.GetY())
    {
      break;
    }
    Drawing::DrawHLine(m_dest,
		       pt->GetY() + y * m_atomDrawSize +
		       m_windowTL.GetY(),
		       pt->GetX() + m_windowTL.GetX(),
		       rightBound,
		       m_gridColor);
  }
}