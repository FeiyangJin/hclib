
import java.util.List;
import java.util.ArrayList;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.io.IOException;

public class TestSmithWaterman extends Test
{
  static {
    theClass = TestSmithWaterman.class;
  }

  String file_name_1 = "../experiments/data/sw_large1.txt";
  String file_name_2 = "../experiments/data/sw_large2.txt";
  int tile_width = 25;
  int tile_height = 25;

  final int GAP_PENALTY = -1;
  final int TRANSITION_PENALTY = -2;
  final int TRANSVERSION_PENALTY = -4;
  final int MATCH = 2;
  final int GAP = 0;

  int[][] alignment_score_matrix =
  {
    {GAP_PENALTY,GAP_PENALTY,GAP_PENALTY,GAP_PENALTY,GAP_PENALTY},
    {GAP_PENALTY,MATCH,TRANSVERSION_PENALTY,TRANSITION_PENALTY,TRANSVERSION_PENALTY},
    {GAP_PENALTY,TRANSVERSION_PENALTY, MATCH,TRANSVERSION_PENALTY,TRANSITION_PENALTY},
    {GAP_PENALTY,TRANSITION_PENALTY,TRANSVERSION_PENALTY, MATCH,TRANSVERSION_PENALTY},
    {GAP_PENALTY,TRANSVERSION_PENALTY,TRANSITION_PENALTY,TRANSVERSION_PENALTY, MATCH}
  };

  byte char_mapping (byte c)
  {
    byte to_be_returned = -1;
    switch(c) {
    case '_': to_be_returned = GAP; break;
    case 'A': to_be_returned = 1; break;
    case 'C': to_be_returned = 2; break;
    case 'G': to_be_returned = 3; break;
    case 'T': to_be_returned = 4; break;
    }
    return to_be_returned;
  }

  static class Tile
  {
    CheckedCompletableFuture<List<Integer>> bottom_row;
    CheckedCompletableFuture<List<Integer>> right_column;
    CheckedCompletableFuture<Integer> bottom_right;
  }

  @Override
  protected void entryPoint (String [] args)
  {
    if (args.length < 4) {
      System.out.println("Using default arguments.");
    } else {
      file_name_1 = args[0];
      file_name_2 = args[1];
      tile_width = Integer.valueOf(args[2]);
      tile_height = Integer.valueOf(args[3]);
    }

    byte[] file_1_tmp = null;
    byte[] file_2_tmp = null;
    try {
      //System.out.println("Reading " + file_name_1);
      file_1_tmp = Files.readAllBytes(Paths.get(file_name_1));
      for (int i = 0; i < file_1_tmp.length; i++)
        file_1_tmp[i] = char_mapping(file_1_tmp[i]);
      //System.out.println("Reading " + file_name_2);
      file_2_tmp = Files.readAllBytes(Paths.get(file_name_2));
      for (int i = 0; i < file_2_tmp.length; i++)
        file_2_tmp[i] = char_mapping(file_2_tmp[i]);
    } catch (IOException e) {
      System.err.println("IOException while reading input files.");
      System.exit(-1);
    }
    final byte[] file_1 = file_1_tmp;
    final byte[] file_2 = file_2_tmp;

    //System.out.println("Tile width is " + tile_width);
    //System.out.println("Tile height is " + tile_height);

    int n_tiles_width = file_1.length/tile_width;
    int n_tiles_height = file_2.length/tile_height;

    //System.out.println("There are " + n_tiles_width + " x " + n_tiles_height + " tiles");

    Tile [][] tile_matrix = new Tile[n_tiles_height+1][n_tiles_width+1];
    for (int i = 0; i < n_tiles_height+1; i++) {
      for (int j = 0; j < n_tiles_width+1; j++) {
        tile_matrix[i][j] = new Tile();
        if (j > 0) tile_matrix[i][j].bottom_row = new CheckedCompletableFuture<>();
        if (i > 0) tile_matrix[i][j].right_column = new CheckedCompletableFuture<>();
        tile_matrix[i][j].bottom_right = new CheckedCompletableFuture<>();
      }
    }

    List<Integer> allocated;
    tile_matrix[0][0].bottom_right.complete(0);
    for (int j = 1; j < n_tiles_width + 1; j++) {
      allocated = new ArrayList<>(tile_width);
      for(int i = 0; i < tile_width ; i++)
        allocated.add(GAP_PENALTY*((j-1)*tile_width+i+1));
      tile_matrix[0][j].bottom_row.complete(allocated);

      tile_matrix[0][j].bottom_right.complete(GAP_PENALTY*(j*tile_width));
    }

    for (int i = 1; i < n_tiles_height + 1; i++) {
      allocated = new ArrayList<>(tile_height);
      for (int j = 0; j < tile_height ; j++)
        allocated.add(GAP_PENALTY*((i-1)*tile_height+j+1));
      tile_matrix[i][0].right_column.complete(allocated);

      tile_matrix[i][0].bottom_right.complete(GAP_PENALTY*(i*tile_height));
    }

    for (int i = 1; i < n_tiles_height+1; i++) {
      for (int j = 1; j < n_tiles_width+1; j++) {
        final int _i = i;
        final int _j = j;
        AnnotatedTask.async(
          tile_matrix[_i][_j].bottom_right,
          tile_matrix[_i][_j].right_column,
          tile_matrix[_i][_j].bottom_row).submit(() -> {
              List<Integer> left_tile_right_column =
                tile_matrix[_i][_j-1].right_column.join();
              List<Integer> above_tile_bottom_row =
                tile_matrix[_i-1][_j].bottom_row.join();
              Integer diagonal_tile_bottom_right =
                tile_matrix[_i-1][_j-1].bottom_right.join();

              int[][] curr_tile = new int[1+tile_height][1+tile_width];
              curr_tile[0][0] = diagonal_tile_bottom_right;
              for (int index = 1; index < tile_height+1; index++)
                curr_tile[index][0] =
                  left_tile_right_column.get(index-1);

              for (int index = 1; index < tile_width+1; index++)
                curr_tile[0][index] =
                  above_tile_bottom_row.get(index-1);

              for (int ii = 1; ii < tile_height+1; ii++) {
                for (int jj = 1; jj < tile_width+1; jj++) {
                  byte char_from_1 = file_1[(_j-1)*tile_width+(jj-1)];
                  byte char_from_2 = file_2[(_i-1)*tile_height+(ii-1)];

                  int diag_score = curr_tile[ii-1][jj-1]
                    + alignment_score_matrix[char_from_2][char_from_1];
                  int left_score = curr_tile[ii  ][jj-1]
                    + alignment_score_matrix[char_from_1][GAP];
                  int  top_score = curr_tile[ii-1][jj  ]
                    + alignment_score_matrix[GAP][char_from_2];

                  int bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
                  curr_tile[ii][jj] =
                    (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;
                }
              }

              Integer curr_bottom_right = curr_tile[tile_height][tile_width];
              tile_matrix[_i][_j].bottom_right.complete(curr_bottom_right);

              List<Integer> curr_right_column = new ArrayList<>(tile_height);
              for (int index = 0; index < tile_height; index++)
                curr_right_column.add(curr_tile[index+1][tile_width]);
              tile_matrix[_i][_j].right_column.complete(curr_right_column);

              List<Integer> curr_bottom_row = new ArrayList<>(tile_width);
              for (int index = 0; index < tile_width; index++)
                curr_bottom_row.add(curr_tile[tile_height][index+1]);
              tile_matrix[_i][_j].bottom_row.complete(curr_bottom_row);
            });
      }
    }

    int score = tile_matrix[n_tiles_height][n_tiles_width].bottom_row.join().get(tile_width-1);
    //System.out.println("score: " + score);
  }
}

