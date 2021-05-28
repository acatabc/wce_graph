import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import sys
import os.path




def clean_data(filename, write_to_csv = True):
    data = pd.read_csv(filename, delimiter=';', index_col=False)

    # remove failed tests
    data = data[data.time.notnull()]

    # rename instances nicely
    data['file'] = data['file'].str.split('/', expand = True).iloc[:, 8].str.split('.', expand = True).iloc[:, 0]

    data.time += 0.01

    if write_to_csv:
        print("generated clean csv")
        fl1 = filename.split('.csv')
        data.to_csv(fl1[0] + '_clean.csv')
    else:
        return data



def generate_compare_dataset(filename1, filename2, write_to_csv = True):
    data1 = pd.read_csv(filename1, delimiter=';', index_col=False)
    data2 = pd.read_csv(filename2, delimiter=';', index_col=False)

    # setup dataframe
    data = [data1["file"], data1["recsteps"], data1["time"], data2["recsteps"], data2["time"]]
    headers = ["file", "recsteps1", "time1", "recsteps2",  "time2"]
    df3 = pd.concat(data, axis=1, keys=headers)

    # set name of instances
    df3['file'] = df3['file'].str.split('/', expand = True).iloc[:, 8].str.split('.', expand = True).iloc[:, 0]

    # drop all instances where both solvers failed
    df3 = df3.dropna(axis=0, thresh = 3)

    # set timeout
    df3.time2.loc[df3.time2.isnull()]  = 300.0
    df3.time1.loc[df3.time1.isnull()]  = 300.0

    # set max rec steps
    rec_max = max(df3.recsteps1.max(), df3.recsteps2.max())
    df3.recsteps2.loc[df3.recsteps2.isnull()]  = rec_max
    df3.recsteps1.loc[df3.recsteps1.isnull()]  = rec_max

    # add time for logplot
    df3.time2 += 0.01
    df3.time1 += 0.01


    if write_to_csv:
        print("generated compare dataset")
        fl1 = filename1.split('.csv')
        fl2 = filename2.split('.csv')
        df3.to_csv(fl1[0] + "_" + fl2[0] + '.csv')
    else:
        return df3



if len(sys.argv) == 2:
    if os.path.isfile(sys.argv[1]):
        clean_data(sys.argv[1])
    else:
        print("no vaild path")
elif len(sys.argv) == 3:
    if os.path.isfile(sys.argv[1]) and os.path.isfile(sys.argv[2]):
        generate_compare_dataset(sys.argv[1], sys.argv[2])
    else:
        print("no vaild path")
else:
    print('type in files as arguments')
