using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


public class Inventory_Item
{
    public string ItemName;

    public string ItemDescription;

    public string ImageSource;

    public int IndexInInventory;

    public Inventory_Item(string Name, string Desc, string Src)
    {
        ItemName = Name;
        ItemDescription = Desc;
        ImageSource = Src;
    }
    
}

