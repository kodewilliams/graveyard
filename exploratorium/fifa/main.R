# install.packages("ggplot2")
# install.packages("dplyr")
# install.packages("rvest")
# install.packages("tidyr")
# install.packages("matrixStats")

library(ggplot2)
library(tidyr)
library(dplyr)
library(rvest)
library(matrixStats)

# Load the FIFA dataset
data_original <- read.csv("data.csv")
data <- read.csv("data.csv")
names(data) <- tolower(names(data))

# Scrape the continent data into a dataframe
continents <- read_html("./continents.html") %>% 
              html_node("table#countries") %>% 
              html_table(header=TRUE, trim=TRUE) %>%
              select("Country", "Continent") %>%
              mutate(Continent = replace_na(Continent, "NA")) %>%
              rename(nationality = Country) %>% data.frame()
names(continents) <- tolower(names(continents))

# Merge datasets using a join
continental_data <- merge(data, continents, by="nationality", all.y=TRUE) %>% data.frame()

# Filter top players by rank into a dataframe by itself
top_players <- data %>% select("age", "overall") %>%
  filter(overall >= 85) %>% data.frame()

# Filter players into a dataframe by itself
players <- data %>% select("age", "overall") %>% data.frame()

# Filter top players by continent into a dataframe by itself
top_players_by_continent <- continental_data %>% select("overall", "nationality", "continent") %>%
                filter(overall >= 85) %>% data.frame()

# Hypothesis #1: Top players are in between the ages 25 - 33.
ggplot(top_players, aes(x=age)) + 
  geom_density(alpha=0.5, fill="aquamarine2", color="aquamarine4") + xlim(16, 45) +
  ggtitle("Hypothesis #1: Top players are in between the ages 25 - 33") +
  theme(plot.title = element_text(hjust = 0.5))

ggplot(players, aes(x=age)) + 
  geom_density(alpha=0.5, fill="aquamarine2", color="aquamarine4") + xlim(16, 45) +
  ggtitle("Distribution of Age Among All Players") +
  theme(plot.title = element_text(hjust = 0.5))

# Hypothesis #2: A majority of the best players are from European countries.
ggplot(top_players_by_continent, aes(x=continent, y=)) +
  geom_bar(alpha=0.5, fill="firebrick1", color="firebrick") +
  ggtitle("Hypothesis #2: A majority of the best players are from European countries") +
  theme(plot.title = element_text(hjust = 0.5))

# Hypothesis #3: Strikers are not good at playing defence.
top_strikers <- data %>% select("position", "lwb", "ldm", "cdm", "rdm", "rwb", "lb", "lcb", "cb", "rcb", "rb", "ls", "st", "rs", "overall") %>%
                filter(position == "ST" | position == "LS" | position == "RS") %>% filter(overall >= 80) %>% data.frame()

strikers_defense <- as.data.frame(sapply(top_strikers %>% 
                    select("lwb", "ldm", "cdm", "rdm", "rwb", "lb", "lcb", "cb", "rcb", "rb"), as.numeric))

strikers_offense <- as.data.frame(sapply(top_strikers %>%
                    select("ls", "st", "rs"), as.numeric))

avg_def_rating <- rowMeans(strikers_defense)
avg_off_rating <- rowMeans(strikers_offense)
avg_all_players_def <- mean(avg_def_rating)
avg_all_players_off <- mean(avg_off_rating)

ggplot(strikers_defense, aes(avg_def_rating)) + 
geom_bar(fill="firebrick1", color="firebrick") + 
  geom_bar(aes(avg_off_rating), fill="aquamarine2", color="aquamarine4") +
  ggtitle("Hypothesis #3: Strikers are not good at playing defense") + 
  xlab("Strikers: Offensive and Defensive Ratings") + ylab("Count")

ggplot(top_strikers, aes(x=avg_all_players_def)) + 
  geom_bar(fill="firebrick1", color="firebrick") + 
  xlim(35, 85) + coord_flip() + 
  geom_bar(aes(x=avg_all_players_off), fill="aquamarine2", color="aquamarine4") +
   ggtitle("Hypothesis #3: Strikers are not good at playing defense") + 
  xlab("Strikers: Offensive and Defensive Ratings Averages") + ylab("Count")

# Hypothesis #4: striking ability is not directly related to freekick ability.
data_4 <- select(data_original, Finishing, FKAccuracy, Overall) %>% filter(overall >= 85) %>% data.frame()
data_4 

ggplot(data_4, aes(x=Finishing)) + geom_histogram()

ggplot(data_4[order(data_4$Finishing),], aes(Finishing, FKAccuracy)) +
  geom_point()

# Basic line plot with points
ggplot(data=data_4 , aes(x=Finishing, y=FKAccuracy, group=1)) +
  geom_line(color="red") + 
  ggtitle("Hypothesis #4: Striking ability is not related to freekick ability")


# Hypothesis #5: Attacking players are paid more than defensive players.
data_5 <- select(data_original, Wage, Position)
# Format data in Wage column.
data_5$Wage <- gsub("[\\€,]", "", data_5$Wage)
data_5$Wage <- as.numeric(gsub("K","000", data_5$Wage))
defensive_players <- filter(data_5,
                            Position == "LWB"|
                            Position == "LDM" |
                            Position == "CDM" |
                            Position == "RDM" |
                            Position == "RWB" |
                            Position == "LB"  |
                            Position == "LCB" |
                            Position == "CB"  |
                            Position == "RCB" |
                            Position == "RB")
attacking_players <- filter(data_5,
                            Position == "LS" | 
                            Position == "ST" | 
                            Position == "RS")

def_mean <- mean(defensive_players$Wage)
off_mean <- mean(attacking_players$Wage)
output <- data.frame("type" = c("Defensive Wage","Offensive Wage"), "wage" = c(def_mean, off_mean))

ggplot(output, aes(x=type, y=wage)) + 
  geom_bar(stat="identity", color="blue", fill="black") +
  geom_text(aes(label=type), vjust=1.6, color="white", size=8) +
  theme_minimal()
