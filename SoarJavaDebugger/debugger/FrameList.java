/********************************************************************************************
*
* FrameList.java
* 
* Description:	
* 
* Created on 	Jan 29, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package debugger;

import java.util.ArrayList;
import java.util.List;

import doc.Module;

/********************************************************************************************
* 
* Used to store a list of all frame windows in the debugger.  (A frame window has a menu bar and
* many children).
* 
********************************************************************************************/
public class FrameList
{
	private List	m_List = new ArrayList() ;
	
	public int	size()				{ return m_List.size() ; }
	public void	add(MainFrame f)	{ m_List.add(f) ; }
	public void remove(MainFrame f) { m_List.remove(f) ; }
	public MainFrame get(int index)	{ return (MainFrame)m_List.get(index) ; }
	
	public int getUnusedChannel()
	{
		throw new IllegalStateException("Not implemented yet") ;
	}
}