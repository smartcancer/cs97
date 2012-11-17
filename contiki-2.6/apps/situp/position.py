class Position(object):
  def __init__(self,x_pos,y_pos,z_pos):
    """
        
    """
    self.x = x_pos
    self.y = y_pos
    self.z = z_pos

  def __str__(self):
    """
        Displays the information about a position as a string.
    """
    msg = "\t\tPosition\n"
    msg += "X: "+string(self.x)
    msg += "\nY: "+string(self.y)
    msg += "\nZ: "+string(self.z)
    return msg
  def X(self):
    return self.x
  def Y(self):
    return self.y
  def Z(self):
    return self.z

