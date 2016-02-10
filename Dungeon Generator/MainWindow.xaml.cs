
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace DungeonGenerator
{

    /// <summary>
    /// This is a Dungeon-building tool based on the algorithm on: http://www.roguebasin.com/index.php?title=Dungeon-Building_Algorithm
    /// I have extended the algorithm by adding bridging support.
    /// </summary>

    public partial class MainWindow : Window
    {
        public string EmptyRoomSymbol;
        public string FloorSymbol;
        public enum Directions
        {
            Up,
            Right,
            Down,
            Left,
            None
        }
        public Directions CandidateConnectingWall;

        public enum TileType
        {
            Square,
            Corridor,
            Rectangle,
            None
        }
        public TileType TileToBuild = TileType.None;

        string FinalOutput;

        public int SquareProbability;
        public int SquareSizeStart;
        public int SquareSizeEnd;
        public int SquareSizeFinal;

        public int CorridorProbability;
        public int CorridorLengthStart;
        public int CorridorLengthEnd;
        public int CorridorLengthFinal;


        public int RectangleProbability;
        public int RecWidthStart;
        public int RecHeightStart;
        public int RecWidthEnd;
        public int RecHeightEnd;

        public int FinalWidth;
        public int FinalHeight;

        private int Y_Index;
        private int X_Index;
        private string[,] LeDungeon;
        private string[,] DungeonFloors;
        private List<int> XIndexList;
        private List<int> YIndexList;
        private int PlayableTiles;

        public List<string> CheckedList;
        private short DungeonWidth;
        private short DungeonHeight;
        private List<Directions> CandidateConnectingWalls;
        private bool RectangleThing;

        public List<int> DungeonOccupiedXs = new List<int>();
        public List<int> DungeonOccupiedYs = new List<int>();


        public int ConnectionChance;
        public int HorizontalChance;
        public int BridgeCount;

        public bool IsWorkerBusy;
        public Dictionary<string, string> MyDic = new Dictionary<string, string>();

        public List<Run> TheRunList = new List<Run>();
        public List<Run> TheGlobalRunList = new List<Run>();

        DispatcherTimer t = new DispatcherTimer();
        private string ProgressionText;
        private double ProgressionValue;
        private int ProgressionValueMax;

        /// <summary>
        /// Initializes the Tool with Values and Grabs Data from UI to the actual variables.
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();

            t.Tick += t_Tick;
            t.Interval = new TimeSpan(1);
            t.Start();

            EmptyRoomSymbol = "X";
            FloorSymbol = "#";
            
            Cor_Prob.Text = "100";
            Cor_Start.Text = "2";
            Cor_End.Text = "4";

            WidthInTiles.Text = "15";
            HeightInTiles.Text = "15";

            PlayableTilesText.Text = "10";

            ConnectionChance_Text.Text = "80";
            HorizontalChanceText.Text = "50";
            BridgeCountText.Text = "10";

            LeUberSlider.Visibility = LeUberSliderText.Visibility = System.Windows.Visibility.Collapsed;
        }

        //////////////////////////////////
        //   Updates the Progress Bar   //
        //////////////////////////////////

        void t_Tick(object sender, EventArgs e)
        {
            LeUberSliderText.Text = ProgressionText + " (" + ProgressionValue + "%)";
            LeUberSlider.Value = ProgressionValue;
        }

        private void GenDungeonBtn_Click(object sender, RoutedEventArgs e)
        {

            #region Validation Check

            if (SQ_End.Text == "0") SQ_End.Text = "1";
            if (SQ_Start.Text == "0") SQ_Start.Text = "1";

            if (Cor_End.Text == "0") Cor_End.Text = "1";
            if (Cor_Start.Text == "0") Cor_Start.Text = "1";

            if (Rec_Width_Start.Text == "0") Rec_Width_Start.Text = "1";
            if (Rec_Width_End.Text == "0") Rec_Width_End.Text = "1";

            if (Rec_Height_End.Text == "0") Rec_Height_End.Text = "1";
            if (Rec_Height_Start.Text == "0") Rec_Height_Start.Text = "1";

            if (Convert.ToInt16(Rec_Height_End.Text) < Convert.ToInt16(Rec_Height_Start.Text)) Rec_Height_End.Text = Rec_Height_Start.Text;
            if (Convert.ToInt16(Rec_Width_End.Text) < Convert.ToInt16(Rec_Width_Start.Text)) Rec_Width_End.Text = Rec_Width_Start.Text;

            if (Convert.ToInt16(Cor_End.Text) < Convert.ToInt16(Cor_Start.Text)) Cor_End.Text = Cor_Start.Text;
            if (Convert.ToInt16(SQ_End.Text) < Convert.ToInt16(SQ_Start.Text)) SQ_End.Text = SQ_Start.Text;

            #endregion

            #region Initialization

            CheckedList = new List<string>();
            
            DungeonWidth = System.Convert.ToInt16(WidthInTiles.Text);
            DungeonHeight = System.Convert.ToInt16(HeightInTiles.Text);
            PlayableTiles = System.Convert.ToInt16(PlayableTilesText.Text);

            SquareProbability = System.Convert.ToInt16(SQ_Prob.Text);
            SquareSizeStart = System.Convert.ToInt16(SQ_Start.Text);
            SquareSizeEnd = System.Convert.ToInt16(SQ_End.Text);

            CorridorProbability = System.Convert.ToInt16(Cor_Prob.Text);
            CorridorLengthStart = System.Convert.ToInt16(Cor_Start.Text);
            CorridorLengthEnd = System.Convert.ToInt16(Cor_End.Text);

            RectangleProbability = System.Convert.ToInt16(RecProb.Text);

            RecHeightEnd = System.Convert.ToInt16(Rec_Height_End.Text);
            RecHeightStart = System.Convert.ToInt16(Rec_Height_Start.Text);
            RecWidthStart = System.Convert.ToInt16(Rec_Width_Start.Text);
            RecWidthEnd = System.Convert.ToInt16(Rec_Width_End.Text);

            BridgeCount = System.Convert.ToInt16(BridgeCountText.Text);
            ConnectionChance = System.Convert.ToInt16(ConnectionChance_Text.Text);
            HorizontalChance = System.Convert.ToInt16(HorizontalChanceText.Text);
            
            
            LeDungeon = new string[DungeonWidth,DungeonHeight];

            
            
            #endregion

            #region Consistency Control
            bool WillIncreaseSQ, WillIncreaseCor, WillIncreaseRec;
            int SQIncrease = 0;
            int RecIncrease = 0;

            if (SquareProbability == CorridorProbability)
            {
                SQIncrease = 1;
            }

            if(SquareProbability == RectangleProbability)
            {
                RecIncrease = 1;
            }

            if(RectangleProbability == CorridorProbability)
            {
                RecIncrease = 1;
            }

            if (RectangleProbability == CorridorProbability && CorridorProbability == SquareProbability)
            {
                RecIncrease = 2;
                SQIncrease = 1;
            }

            SquareProbability += SQIncrease;
            RectangleProbability += RecIncrease;

            int TotalProc = SquareProbability + RectangleProbability + CorridorProbability;

            if(TotalProc>100)
            {
               if(SquareProbability>RectangleProbability && SquareProbability> CorridorProbability)
               {
                   while (SquareProbability + RectangleProbability + CorridorProbability > 100)
                   {
                       SquareProbability--;
                   }
               }
               else if(RectangleProbability>SquareProbability && RectangleProbability>CorridorProbability)
               {
                   while (SquareProbability + RectangleProbability + CorridorProbability > 100)
                   {
                       RectangleProbability--;
                   }
               }
               else if(CorridorProbability>SquareProbability && CorridorProbability>RectangleProbability)
               {
                   while (SquareProbability + RectangleProbability + CorridorProbability > 100)
                   {
                       CorridorProbability--;
                   }
               }
                MessageBox.Show("Fixing Probabilities and recalculating...", "Warning!");
            }
            else if(TotalProc<100)
            {
                if (SquareProbability < RectangleProbability && SquareProbability < CorridorProbability)
                {
                    while (SquareProbability + RectangleProbability + CorridorProbability < 100)
                    {
                        SquareProbability++;
                    }
                }
                else if (RectangleProbability < SquareProbability && RectangleProbability < CorridorProbability)
                {
                    while (SquareProbability + RectangleProbability + CorridorProbability < 100)
                    {
                        RectangleProbability++;
                    }
                }
                else if (CorridorProbability < SquareProbability && CorridorProbability < RectangleProbability)
                {
                    while (SquareProbability + RectangleProbability + CorridorProbability < 100)
                    {
                        CorridorProbability++;
                    }
                }
                MessageBox.Show("Fixing Probabilities and recalculating...", "Warning!");
            }

            


            #endregion

            TheRunList = new List<Run>();

            DungeonOccupiedXs = new List<int>();
            DungeonOccupiedYs = new List<int>();

            SQ_Prob.Text = SquareProbability.ToString();
            RecProb.Text = RectangleProbability.ToString();
            Cor_Prob.Text = CorridorProbability.ToString();


            BackgroundWorker AsyncGenerator = new BackgroundWorker();
            AsyncGenerator.DoWork += AsyncGenerator_DoWork;
            AsyncGenerator.RunWorkerCompleted += AsyncGenerator_RunWorkerCompleted;

            IsWorkerBusy = true;
            GenDungeonBtn.IsEnabled = false;
            SaveDungeonBtn.IsEnabled = false;
            AsyncGenerator.RunWorkerAsync();

            LeUberSlider.Visibility = LeUberSliderText.Visibility = System.Windows.Visibility.Visible;

            Loader.Visibility = System.Windows.Visibility.Visible;
 
        }

        void AsyncGenerator_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            DungeonVisual.Text = "";
            
            SolidColorBrush EmptyColor = new SolidColorBrush(Colors.Red);
            SolidColorBrush FilledColor = new SolidColorBrush(Color.FromArgb(255, 0, 255, 0));

            ProgressionText = "Finalizing... ";
            ProgressionValue = 0;
            ProgressionValueMax = FinalOutput.Length;
            int Iterations = 0;

            for (int i = 0; i < FinalOutput.Length; i++)
            {
                if (FinalOutput[i].ToString() == EmptyRoomSymbol)
                {
                    Run run = new Run("■");
                    run.Foreground = EmptyColor;
                    TheRunList.Add(run);

                }
                else if (FinalOutput[i].ToString() == FloorSymbol)
                {
                    Run run = new Run("■");
                    run.Foreground = FilledColor;
                    TheRunList.Add(run);
                }
                else if (FinalOutput[i].ToString() == "-")
                {
                    Run run = new Run(Environment.NewLine);
                    TheRunList.Add(run);

                }

                Iterations++;
                ProgressionValue = (Iterations / ProgressionValueMax) * 100;
            }
            
            foreach (var item in TheRunList)
            {
                DungeonVisual.Inlines.Add(item);
            }

            DungeonVisual.FontSize = 18;
            DungeonVisual.LineHeight = 12f;

            DungeonVisual.LineStackingStrategy = LineStackingStrategy.BlockLineHeight;

            LeUberSlider.Visibility = LeUberSliderText.Visibility = System.Windows.Visibility.Collapsed;

            IsWorkerBusy = false;
            Loader.Visibility = System.Windows.Visibility.Hidden;
            MessageBox.Show("Your " + DungeonWidth + "x" + DungeonHeight + " Dungeon has been genereated!", "Notification");
            GenDungeonBtn.IsEnabled = true;
            SaveDungeonBtn.IsEnabled = true;
                
            
        }
        
        /// <summary>
        /// A Heavy, loop-intensive part that when runs on another thread (Background Agent) gives us the ability to:
        /// 1 - feed data back to the progress bar
        /// 2 - Bypass the freezing due to heavy overload on main thread.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void AsyncGenerator_DoWork(object sender, DoWorkEventArgs e)
        {
            FinalOutput = "";

            List<Directions> PingedDirections = new List<Directions>();

            ProgressionText = "Populating the Room";
            ProgressionValue = 0;
            ProgressionValueMax = DungeonHeight * DungeonWidth;

            int Iterations = 0;

            #region Fill the room with solid path.
            for (int i = 0; i < DungeonHeight; i++)
            {
                for (int j = 0; j < DungeonWidth; j++)
                {
                    LeDungeon[j, i] = EmptyRoomSymbol;
                    Iterations++;
                    ProgressionValue = (Iterations / ProgressionValueMax) * 100;
                }
            }
            #endregion

            #region Select a random point of the map as a starting point

            ProgressionText = "Selecting a Random Node";
            ProgressionValue = 0;
            ProgressionValueMax = 1;

            Random R = new Random();
            int RandomX = R.Next(0, DungeonWidth);
            int RandomY = R.Next(0, DungeonHeight);

            LeDungeon[RandomX, RandomY] = FloorSymbol;
            DungeonOccupiedXs.Add(RandomX);
            DungeonOccupiedYs.Add(RandomY);

            ProgressionValue = 0;


            #endregion

            #region Add New Rooms until all given tiles are created or the dungeon is complete

            ProgressionText = "Adding Rooms";
            ProgressionValue = 0;

            double PlacedTiles = 0;
            double TilesToPlace = PlayableTiles;

            ProgressionValueMax = PlayableTiles;
            
            while (PlayableTiles > 0)
            {

                X_Index = -1;
                Y_Index = -1;

                bool WillBreak = false;

                
                    for (int j = 0; j < DungeonOccupiedXs.Count; j++)
                    {
                        if (LeDungeon[DungeonOccupiedXs[j], DungeonOccupiedYs[j]] == FloorSymbol &&
                            (!CheckedList.Contains(DungeonOccupiedXs[j].ToString() + "," + DungeonOccupiedYs[j].ToString()))
                            )
                        {
                            X_Index = DungeonOccupiedXs[j];
                            Y_Index = DungeonOccupiedYs[j];

                            WillBreak = true;

                        }

                        if (WillBreak)
                        {
                            break;
                        }
                    }
                

                if (X_Index == -1 && Y_Index == -1)
                {
                    // I Haven't found anything. Breaking the loop.
                    MessageBox.Show("Not enough space to fill the Dungeon with the selected room types!");
                    break;
                }
                else
                {
                    CandidateConnectingWalls = new List<Directions>();
                    PingedDirections = new List<Directions>();
                    CandidateConnectingWall = Directions.None;

                    FindPossibleConnectors();


                    if (CandidateConnectingWalls.Count == 0)
                    {
                        CheckedList.Add(X_Index.ToString() + "," + Y_Index.ToString());
                        continue;
                    }

                    // Determine what to build
                    DetermineTileType(R);

                    //Randomize values based on candidate Tile and define final size
                    RandomizeValues(R);

                    XIndexList = new List<int>();
                    YIndexList = new List<int>();

                    bool CanIPlaceThisThing = PositionChecker();

                    // If we can place it, do so. Else go back to step 3.
                    if (CanIPlaceThisThing)
                    {
                        PlaceTheTile(XIndexList, YIndexList);
                        PlayableTiles--;
                        PlacedTiles++;
                       
                        CheckedList = new List<string>();
                        ProgressionValue = Math.Round((PlacedTiles / TilesToPlace),2) * 100;
                    }
                    else
                    {
                        CheckedList.Add(X_Index.ToString() + "," + Y_Index.ToString());
                    }
                }
            }
            #endregion

            #region Wrapping Up

            ProgressionText = "Connecting Corridors... ";

            ConnectAFewRooms(BridgeCount);

            #endregion

            #region Parse the Dungeon

            ProgressionText = "Parsing the Dungeon... ";
            ProgressionValue = 0;
            ProgressionValueMax = DungeonHeight * DungeonWidth;
            Iterations = 0;

            for (int i = 0; i < DungeonHeight; i++)
            {
                for (int j = 0; j < DungeonWidth; j++)
                {

                   // var X = new TextBlock();
                   // X.Foreground = (LeDungeon)
            

                    FinalOutput += LeDungeon[j, i] + " ";
                    if (j == DungeonWidth - 1) FinalOutput += "-";

                    Iterations++;
                    ProgressionValue = (Iterations / ProgressionValueMax) * 100;
                }
            }

            

           // TextRange X = new TextRange(DungeonFinalVisual.Document.ContentStart, DungeonFinalVisual.Document.ContentEnd);
           // DungeonFinalVisual.

            
            #endregion
             
        }

        private void FindPossibleConnectors()
        {
            if (DoesThisExist(X_Index, Y_Index - 1))
            {
                if (!IsThisTileOccupied(X_Index, Y_Index - 1))
                {
                    CandidateConnectingWalls.Add(Directions.Up);
                }
            }
            if (DoesThisExist(X_Index + 1, Y_Index))
            {
                if (!IsThisTileOccupied(X_Index + 1, Y_Index))
                {
                    CandidateConnectingWalls.Add(Directions.Right);
                }

            }
            if (DoesThisExist(X_Index, Y_Index + 1))
            {
                if (!IsThisTileOccupied(X_Index, Y_Index + 1))
                {
                    CandidateConnectingWalls.Add(Directions.Down);
                }
            }


            if (DoesThisExist(X_Index - 1, Y_Index))
            {
                if (!IsThisTileOccupied(X_Index - 1, Y_Index))
                {
                    CandidateConnectingWalls.Add(Directions.Left);
                }
            }

        }

        private void PlaceTheTile(List<int> XIndexList, List<int> YIndexList)
        {
            for (int i = 0; i < XIndexList.Count; i++)
            {
                LeDungeon[XIndexList[i], YIndexList[i]] = FloorSymbol;
                DungeonOccupiedXs.Add(XIndexList[i]);
                DungeonOccupiedYs.Add(YIndexList[i]);
            } 
        }

        /// <summary>
        /// Randomizes the room shape based on the values given on the tool.
        /// </summary>
        /// <param name="R"></param>
        private void RandomizeValues(Random R)
        {
            int Proc = R.Next(1, 3);
            bool WillReverse = (Proc == 1);
            
            switch (TileToBuild)
            {
                case TileType.Square:
                    SquareSizeFinal = R.Next(SquareSizeStart, SquareSizeEnd + 1);
                    FinalWidth = FinalHeight = SquareSizeFinal;
                    break;
                case TileType.Corridor:
                    CorridorLengthFinal = R.Next(CorridorLengthStart, CorridorLengthEnd + 1);
                    break;
                case TileType.Rectangle:
                    int TempWidth = R.Next(RecWidthStart, RecWidthEnd + 1) - 1;
                    int TempHeight = R.Next(RecHeightStart, RecHeightEnd + 1) - 1;
                    FinalHeight = (WillReverse) ? TempWidth : TempHeight;
                    FinalWidth = (WillReverse) ? TempHeight : TempWidth;
                    break;
                case TileType.None:
                    break;
                default:
                    break;
            }

            
        }

        /// <summary>
        /// Determines the tile type to look for next.
        /// </summary>
        /// <param name="R"></param>
        private void DetermineTileType(Random R)
        {
            int Proc = R.Next(0, 100);
            Proc += 1;

            if (Proc <= SquareProbability && Proc > 0)
            {
                TileToBuild = TileType.Square;
                RectangleThing = false;
            }
            else if (Proc > SquareProbability && Proc <= CorridorProbability+SquareProbability)
            {
                TileToBuild = TileType.Corridor;
                RectangleThing = false;
            }
            else if (Proc > CorridorProbability + SquareProbability && Proc <= 100)
            {
                RectangleThing = true;
                TileToBuild = TileType.Rectangle;
            }
        }

        /// <summary>
        /// Backbone of the App, checking if we can add a tile in the selected position.
        /// </summary>
        /// <returns></returns>
        private bool PositionChecker()
        {
            bool Test;

            bool WillContinueWithNextBig = true;
            while (CandidateConnectingWalls.Count > 0)
            {
                CandidateConnectingWall = CandidateConnectingWalls.First();
                CandidateConnectingWalls.Remove(CandidateConnectingWalls.First());

                switch (TileToBuild)
                {
                    case TileType.Square:
                        break;
                    case TileType.Corridor:

                        if (CandidateConnectingWall == Directions.Down || CandidateConnectingWall == Directions.Up)
                        {
                            FinalWidth = 1;
                            FinalHeight = CorridorLengthFinal;
                        }
                        else if (CandidateConnectingWall == Directions.Right || CandidateConnectingWall == Directions.Left)
                        {
                            FinalHeight = 1;
                            FinalWidth = CorridorLengthFinal;
                        }

                        break;
                    case TileType.Rectangle:
                        break;
                    case TileType.None:
                        break;
                    default:
                        break;
                }

                #region Dungeon Carver

                    switch (CandidateConnectingWall)
                    {
                        case Directions.Up:
                            // Upper Room Check.
                            // If this try succeeds, then we can create a connecting room.
                            Test = DoesThisExist(X_Index, Y_Index - 1);
                            if (!Test) return false;

                            //Upper Room
                            if (!IsThisTileOccupied(X_Index, Y_Index - 1))
                            {
                                int XStartIndex = X_Index - (FinalWidth - 1);

                                //The Room is empty. 
                                //Continue, checking if the tile exists.
                                for (int X = 0; X < FinalWidth; X++)
                                {
                                    if (WillContinueWithNextBig)
                                    {
                                        WillContinueWithNextBig = false;

                                        XIndexList = new List<int>();
                                        YIndexList = new List<int>();

                                        XIndexList.Add(X_Index);
                                        YIndexList.Add(Y_Index - 1);

                                        for (int i = 0; i < FinalWidth; i++)
                                        {
                                            for (int j = 0; j < FinalHeight; j++)
                                            {
                                                if (DoesThisExist(XStartIndex + i + X, Y_Index - 2 - j))
                                                {
                                                    if (!IsThisTileOccupied(XStartIndex + i + X, Y_Index - 2 - j))
                                                    {
                                                        // Get in here, every time, and you are ok.
                                                        if (!DoesThisHaveNeighbours(XStartIndex + i + X, Y_Index - 2 - j) && !WillContinueWithNextBig)
                                                        {
                                                            XIndexList.Add(XStartIndex + i + X);
                                                            YIndexList.Add(Y_Index - 2 - j);
                                                        }
                                                        else
                                                        {
                                                            WillContinueWithNextBig = true;
                                                            XIndexList = new List<int>();
                                                            YIndexList = new List<int>();
                                                        }
                                                    }
                                                    else
                                                    {
                                                        WillContinueWithNextBig = true;
                                                        XIndexList = new List<int>();
                                                        YIndexList = new List<int>();
                                                    }
                                                }
                                                else
                                                {
                                                    WillContinueWithNextBig = true;
                                                    XIndexList = new List<int>();
                                                    YIndexList = new List<int>();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case Directions.Right:
                            // Upper Room Check.
                            // If this try succeeds, then we can create a connecting room.
                            Test = DoesThisExist(X_Index + 1, Y_Index);
                            if (!Test) return false;

                            //Upper Room
                            if (!IsThisTileOccupied(X_Index + 1, Y_Index))
                            {
                                int YStartIndex = Y_Index - (FinalHeight - 1);

                                //The Room is empty. 
                                //Continue, checking if the tile exists.
                                for (int X = 0; X < FinalHeight; X++)
                                {
                                    if (WillContinueWithNextBig)
                                    {
                                        WillContinueWithNextBig = false;

                                        XIndexList = new List<int>();
                                        YIndexList = new List<int>();

                                        XIndexList.Add(X_Index + 1);
                                        YIndexList.Add(Y_Index);

                                        for (int i = 0; i < FinalWidth; i++)
                                        {
                                            for (int j = 0; j < FinalHeight; j++)
                                            {
                                                if (DoesThisExist(X_Index + i + 2, YStartIndex + j + X))
                                                {
                                                    if (!IsThisTileOccupied(X_Index + i + 2, YStartIndex + j + X))
                                                    {
                                                        // Get in here, every time, and you are ok.
                                                        if (!DoesThisHaveNeighbours(X_Index + i + 2, YStartIndex + j + X) && !WillContinueWithNextBig)
                                                        {
                                                            XIndexList.Add(X_Index + i + 2);
                                                            YIndexList.Add(YStartIndex + j + X);
                                                        }
                                                        else
                                                        {
                                                            WillContinueWithNextBig = true;
                                                            XIndexList = new List<int>();
                                                            YIndexList = new List<int>();
                                                        }
                                                    }
                                                    else
                                                    {
                                                        WillContinueWithNextBig = true;
                                                        XIndexList = new List<int>();
                                                        YIndexList = new List<int>();
                                                    }
                                                }
                                                else
                                                {
                                                    WillContinueWithNextBig = true;
                                                    XIndexList = new List<int>();
                                                    YIndexList = new List<int>();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case Directions.Down:
                            // Upper Room Check.
                            // If this try succeeds, then we can create a connecting room.
                            Test = DoesThisExist(X_Index, Y_Index + 1);
                            if (!Test) return false;

                            //Upper Room
                            if (!IsThisTileOccupied(X_Index, Y_Index + 1))
                            {
                                int XStartIndex = X_Index - (FinalWidth - 1);

                                //The Room is empty. 
                                //Continue, checking if the tile exists.
                                for (int X = 0; X < FinalWidth; X++)
                                {
                                    if (WillContinueWithNextBig)
                                    {
                                        WillContinueWithNextBig = false;

                                        XIndexList = new List<int>();
                                        YIndexList = new List<int>();

                                        XIndexList.Add(X_Index);
                                        YIndexList.Add(Y_Index + 1);


                                        for (int i = 0; i < FinalWidth; i++)
                                        {
                                            for (int j = 0; j < FinalHeight; j++)
                                            {
                                                if (DoesThisExist(XStartIndex + i + X, Y_Index + 2 + j))
                                                {
                                                    if (!IsThisTileOccupied(XStartIndex + i + X, Y_Index + 2 + j))
                                                    {
                                                        // Get in here, every time, and you are ok.

                                                        if (!DoesThisHaveNeighbours(XStartIndex + i + X, Y_Index + 2 + j) && !WillContinueWithNextBig)
                                                        {
                                                            XIndexList.Add(XStartIndex + i + X);
                                                            YIndexList.Add(Y_Index + 2 + j);
                                                        }
                                                        else
                                                        {
                                                            WillContinueWithNextBig = true;
                                                            XIndexList = new List<int>();
                                                            YIndexList = new List<int>();
                                                        }

                                                    }
                                                    else
                                                    {
                                                        WillContinueWithNextBig = true;
                                                        XIndexList = new List<int>();
                                                        YIndexList = new List<int>();
                                                    }
                                                }
                                                else
                                                {
                                                    WillContinueWithNextBig = true;
                                                    XIndexList = new List<int>();
                                                    YIndexList = new List<int>();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case Directions.Left:
                            // Upper Room Check.
                            // If this try succeeds, then we can create a connecting room.
                            Test = DoesThisExist(X_Index - 1, Y_Index);
                            if (!Test) return false;

                            //Upper Room
                            if (!IsThisTileOccupied(X_Index - 1, Y_Index))
                            {
                                int YStartIndex = Y_Index - (FinalHeight - 1);
                                for (int X = 0; X < FinalHeight; X++)
                                {
                                    if (WillContinueWithNextBig)
                                    {
                                        WillContinueWithNextBig = false;

                                        XIndexList = new List<int>();
                                        YIndexList = new List<int>();

                                        XIndexList.Add(X_Index - 1);
                                        YIndexList.Add(Y_Index);

                                        for (int i = 0; i < FinalWidth; i++)
                                        {
                                            for (int j = 0; j < FinalHeight; j++)
                                            {
                                                if (DoesThisExist(X_Index - i - 2, YStartIndex + j + X))
                                                {
                                                    if (!IsThisTileOccupied(X_Index - i - 2, YStartIndex + j + X))
                                                    {
                                                        // Get in here, every time, and you are ok.
                                                        if (!DoesThisHaveNeighbours(X_Index - i - 2, YStartIndex + j + X) && !WillContinueWithNextBig)
                                                        {
                                                            XIndexList.Add(X_Index - i - 2);
                                                            YIndexList.Add(YStartIndex + j + X);
                                                        }
                                                        else
                                                        {
                                                            WillContinueWithNextBig = true;
                                                            XIndexList = new List<int>();
                                                            YIndexList = new List<int>();
                                                        }
                                                    }
                                                    else
                                                    {
                                                        WillContinueWithNextBig = true;
                                                        XIndexList = new List<int>();
                                                        YIndexList = new List<int>();
                                                    }
                                                }
                                                else
                                                {
                                                    WillContinueWithNextBig = true;
                                                    XIndexList = new List<int>();
                                                    YIndexList = new List<int>();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        case Directions.None:
                            break;
                        default:
                            break;
                    }
                
                #endregion

                if (XIndexList.Count > 0 && YIndexList.Count > 0)
                {
                    break;
                }
                
            }
            
            return (XIndexList.Count > 0 && YIndexList.Count > 0);
        }

        #region Main Helpers

        private bool DoesThisExist(int X, int Y)
        {
            try
            {
                string TestString = LeDungeon[X, Y];
            }
            catch (IndexOutOfRangeException e)
            {
                return false;
            }
            return true;
        }

        private bool IsThisTileOccupied(int X, int Y)
        {
            return (LeDungeon[X, Y] == FloorSymbol);
        }

        private bool DoesThisHaveNeighbours(int X,int Y)
        {
            if (DoesThisExist(X - 1, Y))
            {
                if (IsThisTileOccupied(X - 1, Y))
                {
                    return true;
                }
            }
            if (DoesThisExist(X + 1, Y))
            {
                if (IsThisTileOccupied(X + 1, Y))
                {
                    return true;
                }
            }
            if (DoesThisExist(X, Y - 1))
            {
                if (IsThisTileOccupied(X, Y - 1))
                {
                    return true;
                }
            }
            if (DoesThisExist(X, Y + 1))
            {
                if (IsThisTileOccupied(X, Y + 1))
                {
                    return true;
                }
            }

            return false;
        }

        private bool IsThereAVacantNeighboor(int X,int Y)
        {
            if (DoesThisExist(X - 1, Y))
            {
                if (!IsThisTileOccupied(X - 1, Y))
                {
                    return true;
                }
            }
            if (DoesThisExist(X + 1, Y))
            {
                if (!IsThisTileOccupied(X + 1, Y))
                {
                    return true;
                }
            }
            if (DoesThisExist(X, Y - 1))
            {
                if (!IsThisTileOccupied(X, Y - 1))
                {
                    return true;
                }
            }
            if (DoesThisExist(X, Y + 1))
            {
                if (!IsThisTileOccupied(X, Y + 1))
                {
                    return true;
                }
            }

            return false;
        }

        #endregion

        /// <summary>
        /// Bridging Feature.
        /// </summary>
        /// <param name="Bridges"></param>
        private void ConnectAFewRooms(int Bridges)
        {
            Random R = new Random();

            double BridgesToBuild=Bridges;
            double BridgesBuilt=0;

            int Iterations = 0;
            while (Iterations < 3)
            {
                for (int j = 0; j < DungeonOccupiedYs.Count; j++)
                {

                    if (LeDungeon[DungeonOccupiedXs[j], DungeonOccupiedYs[j]] == FloorSymbol)
                        {

                            int Proc = R.Next(0, 100) + 1;
                            if (Proc <= HorizontalChance)
                            {
                                if (DoesThisExist(DungeonOccupiedXs[j] + 1, DungeonOccupiedYs[j]))
                                {
                                    if (LeDungeon[DungeonOccupiedXs[j] + 1, DungeonOccupiedYs[j]] == EmptyRoomSymbol)
                                    {
                                        Proc = R.Next(0, 100) + 1;
                                        if (Proc <= ConnectionChance)
                                        {
                                            int Length = GetRoomAcrossHoriz(DungeonOccupiedXs[j], DungeonOccupiedYs[j]);
                                            if (Length > 0)
                                            {
                                                for (int w = 0; w < Length; w++)
                                                {
                                                    LeDungeon[DungeonOccupiedXs[j] + w + 1, DungeonOccupiedYs[j]] = FloorSymbol;
                                                }
                                                Bridges--;
                                                BridgesBuilt++;
                                                if (Bridges <= 0) return;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if (DoesThisExist(DungeonOccupiedXs[j], DungeonOccupiedYs[j] + 1))
                                {
                                    if (LeDungeon[DungeonOccupiedXs[j], DungeonOccupiedYs[j] + 1] == EmptyRoomSymbol)
                                    {
                                        Proc = R.Next(0, 100) + 1;
                                        if (Proc <= ConnectionChance)
                                        {
                                            int Length = GetRoomAcrossVertic(DungeonOccupiedXs[j], DungeonOccupiedYs[j]);
                                            if (Length > 0)
                                            {
                                                for (int w = 0; w < Length; w++)
                                                {
                                                    LeDungeon[DungeonOccupiedXs[j], DungeonOccupiedYs[j] + 1 + w] = FloorSymbol;
                                                }
                                                Bridges--;
                                                BridgesBuilt++;
                                                if (Bridges <= 0) return;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                
                Iterations++;
                ProgressionValue = Math.Round((BridgesBuilt / BridgesToBuild), 2) * 100;
            }
            ProgressionValue = 100;
        }

        #region Bridging Feature Helpers

        private int GetRoomAcrossHoriz(int X, int Y)
        {
            int Length = 0;
            bool WillBreak = false;
            while (!WillBreak)
            {
                if (DoesThisExist(X+1, Y) && DoesThisExist(X + 1, Y -1) && DoesThisExist(X + 1, Y + 1)
                    && DoesThisExist(X, Y+1) && DoesThisExist(X, Y-1))
                {

                    if (Length == 0)
                    {
                        //an to epomeno mou einai gemato, #tapame.
                        if (IsThisTileOccupied(X+1, Y) || IsThisTileOccupied(X + 1, Y + 1) || IsThisTileOccupied(X + 1, Y - 1))
                        {
                            return Length;
                        }
                        else
                        {
                            Length++;
                            X++;

                            if (IsThisTileOccupied(X, Y+1) || IsThisTileOccupied(X, Y - 1))
                            {
                                return Length;
                            }
                        }
                    }
                    else if (IsThisTileOccupied(X + 1, Y))
                    {
                        return Length;
                    }
                    
                    else
                    {
                        Length++;
                        X++;

                        if (IsThisTileOccupied(X, Y + 1) || IsThisTileOccupied(X, Y - 1))
                        {
                            return Length;
                        }
                    }
                }
                else
                {
                    return 0;
                }
            
            }
            return Length;
        }

        private int GetRoomAcrossVertic(int X, int Y)
        {
            int Length = 0;
            bool WillBreak = false;
            while (!WillBreak)
            {
                if (DoesThisExist(X, Y+1) && DoesThisExist(X - 1, Y +1) && DoesThisExist(X + 1, Y + 1)
                    && DoesThisExist(X+1, Y) && DoesThisExist(X-1, Y))
                {

                    if (Length == 0)
                    {
                        //an to epomeno mou einai gemato, #tapame.
                        if (IsThisTileOccupied(X, Y+1) || IsThisTileOccupied(X + 1, Y + 1) || IsThisTileOccupied(X - 1, Y + 1))
                        {
                            return Length;
                        }
                        else
                        {
                            Length++;
                            Y++;

                            if (IsThisTileOccupied(X+1, Y) || IsThisTileOccupied(X-1,Y))
                            {
                                return Length;
                            }
                        }
                    }
                    else if (IsThisTileOccupied(X, Y+1))
                    {
                        return Length;
                    }

                    else
                    {
                        Length++;
                        Y++;

                        if (IsThisTileOccupied(X+1, Y) || IsThisTileOccupied(X-1, Y))
                        {
                            return Length;
                        }
                    }
                }
                else
                {
                    return 0;
                }
            }
            return Length;
        }

        #endregion

        #region Export Function

        private void SaveDungeonBtn_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();
            dlg.FileName = "My_New_Dungeon"; // Default file name
            dlg.DefaultExt = ".text"; // Default file extension
            dlg.Filter = "Dok Dungeon|*.txt"; // Filter files by extension

            // Show save file dialog box
            Nullable<bool> result = dlg.ShowDialog();

            // Process save file dialog box results
            if (result == true)
            {
                // Save document
                string filename = dlg.FileName;
                System.IO.File.WriteAllText(filename, DungeonWidth+";"+DungeonHeight+";"+FinalOutput);
                MessageBox.Show("Your dungeon has been successfully saved!", "Notification");

            }
            //// Displays a SaveFileDialog so the user can save the Image
            //// assigned to Button2.
            //SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            //saveFileDialog1.Filter = "Dok Dungeon File|*.txt";
            //saveFileDialog1.Title = "Save the Dungeon";
            //bool ?res = saveFileDialog1.ShowDialog();

            //if ((bool)res)
            //{
            //    // If the file name is not an empty string open it for saving.
            //    if (saveFileDialog1.FileName != "")
            //    {
            //        System.IO.FileStream fs =
            //           (System.IO.FileStream)saveFileDialog1.OpenFile();
            //        switch (saveFileDialog1.FilterIndex)
            //        {
            //            case 1:

            //                System.IO.File.WriteAllText(saveFileDialog1.FileName, FinalOutput);
            //                break;
            //        }

            //        fs.Close();
            //    }
            //}
            
        }

        #endregion

        #region Validation Check

        private void GotFocus(object sender, RoutedEventArgs e)
        {
            var controller = (TextBox)sender;

            if (!MyDic.ContainsKey(controller.Name))
                MyDic.Add(controller.Name, controller.Text);
            else
                MyDic[controller.Name] = controller.Text;

            controller.Text = "";
        }

        private void LoseFocus(object sender, RoutedEventArgs e)
        {
            var controller = (TextBox)sender;

            if (controller.Text == "")
            {
                if (MyDic.ContainsKey(controller.Name))
                    controller.Text = MyDic[controller.Name];
                else
                    controller.Text = "0";
            }
        }

        private void PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            var controller = (TextBox)sender;
            if (controller.Text.Length > 2)
            {
                controller.Text = controller.Text.Substring(0, 3);
                e.Handled = true;
                return;
            }
             e.Handled = new Regex("[^0-9]").IsMatch(e.Text);

        }

        #endregion
    }
}
