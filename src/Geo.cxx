#include "Geo.h"

Geo::Geo()
{
  m_plane[BiLayerView(0, 1)] = 0;
  m_plane[BiLayerView(0, 0)] = 1;
  m_plane[BiLayerView(1, 0)] = 2;
  m_plane[BiLayerView(1, 1)] = 3;
  m_plane[BiLayerView(2, 1)] = 4;
  m_plane[BiLayerView(2, 0)] = 5;
  m_plane[BiLayerView(3, 0)] = 6;
  m_plane[BiLayerView(3, 1)] = 7;
  m_plane[BiLayerView(4, 1)] = 8;
  m_plane[BiLayerView(4, 0)] = 9;
  m_plane[BiLayerView(5, 0)] = 10;
  m_plane[BiLayerView(5, 1)] = 11;
  m_plane[BiLayerView(6, 1)] = 12;
  m_plane[BiLayerView(6, 0)] = 13;
  m_plane[BiLayerView(7, 0)] = 14;
  m_plane[BiLayerView(7, 1)] = 15;
  m_plane[BiLayerView(8, 1)] = 16;
  m_plane[BiLayerView(8, 0)] = 17;
  m_plane[BiLayerView(9, 0)] = 18;
  m_plane[BiLayerView(9, 1)] = 19;
  m_plane[BiLayerView(10, 1)] = 20;
  m_plane[BiLayerView(10, 0)] = 21;
  m_plane[BiLayerView(11, 0)] = 22;
  m_plane[BiLayerView(11, 1)] = 23;
  m_plane[BiLayerView(12, 1)] = 24;
  m_plane[BiLayerView(12, 0)] = 25;
  m_plane[BiLayerView(13, 0)] = 26;
  m_plane[BiLayerView(13, 1)] = 27;
  m_plane[BiLayerView(14, 1)] = 28;
  m_plane[BiLayerView(14, 0)] = 29;
  m_plane[BiLayerView(15, 0)] = 30;
  m_plane[BiLayerView(15, 1)] = 31;
  m_plane[BiLayerView(16, 1)] = 32;
  m_plane[BiLayerView(16, 0)] = 33;
  m_plane[BiLayerView(17, 0)] = 34;
  m_plane[BiLayerView(17, 1)] = 35;
}

