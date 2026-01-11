import matplotlib.pyplot as plt
import pandas as pd


def main():
    print("Hello world!")

    df = pd.read_csv("./data/local.csv")

    plt.figure(figsize=(10, 5))
    plt.plot(df["workers"], df["time"], marker="o")
    plt.title("Job Dispatcher Benchmark")
    plt.xlabel("Number of Workers")
    plt.ylabel("Time (seconds)")
    plt.grid()
    plt.savefig("./plots/local_time.png")
    plt.show()

    serial_time = df.loc[df["workers"] == 1, "time"].iloc[0]

    df["speedup"] = serial_time / df["time"]
    plt.figure(figsize=(10, 5))
    plt.plot(df["workers"], df["speedup"], marker="o", color="orange")
    plt.title("Job Dispatcher Speedup")
    plt.xlabel("Number of Workers")
    plt.ylabel("Speedup")
    plt.grid()
    plt.savefig("./plots/local_speedup.png")
    plt.show()

    df["efficiency"] = df["speedup"] / df["workers"]
    plt.figure(figsize=(10, 5))
    plt.plot(df["workers"], df["efficiency"], marker="o", color="green")
    plt.title("Job Dispatcher Efficiency")
    plt.xlabel("Number of Workers")
    plt.ylabel("Efficiency")
    plt.grid()
    plt.savefig("./plots/local_efficiency.png")
    plt.show()


if __name__ == "__main__":
    main()
